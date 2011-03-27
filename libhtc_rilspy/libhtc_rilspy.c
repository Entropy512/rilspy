/*
 */
#include <telephony/ril.h>
#include <dlfcn.h>

#include <utils/Log.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/wait.h>
#include <termios.h>
#include <time.h>

#undef LOG_TAG
#define LOG_TAG "RIL_SPY"

static void s_onRequestComplete(RIL_Token t, RIL_Errno e, void *r, size_t rlen);
static void s_onUnsolicitedResponse(int res, const void *r, size_t rlen);
static void s_requestTimedCallback(RIL_TimedCallback cb, void *p, const struct timeval *tv);

static void s_onRequest(int r, void *d, size_t dlen, RIL_Token t);
static RIL_RadioState s_onStateRequest();
static int s_supports(int r);
static void s_onCancel(RIL_Token t);

static const struct RIL_Env s_rilenv = {
	s_onRequestComplete,
	s_onUnsolicitedResponse,
	s_requestTimedCallback
}, *h_rilenv;

static RIL_RadioFunctions s_radiofunctions = {
	0,
	s_onRequest,
	s_onStateRequest,
	s_supports,
	s_onCancel,
	0
};
static const RIL_RadioFunctions *h_radiofunctions;

int s_fd;	/* real /dev/smd0 */
int p_fd;	/* pty */

static void s_onRequestComplete(RIL_Token t, RIL_Errno e, void *r, size_t rlen)
{
	LOGD("s_onRequestComplete(%p, %d, %p, %d)\n", t, e, r, rlen);
    h_rilenv->OnRequestComplete(t, e, r, rlen);
}

static void s_onUnsolicitedResponse(int u, const void *r, size_t rlen)
{
    LOGD("s_onUnsolicitedResponse(%d, %p, %u)", u, r, rlen);
    h_rilenv->OnUnsolicitedResponse(u, r, rlen);
}

static void s_requestTimedCallback(RIL_TimedCallback cb, void *p, const struct timeval *tv)
{
    LOGD("s_requestTimedCallback(%p, %p, {%ld,%ld})\n", cb, p, tv->tv_sec, tv->tv_usec);
    h_rilenv->RequestTimedCallback(cb, p, tv);
}

static void s_onRequest(int r, void *d, size_t dlen, RIL_Token t)
{
	LOGD("s_onRequest(%d, %p, %d, %p)\n", r, d, dlen, t);
	h_radiofunctions->onRequest(r, d, dlen, t);
}

static RIL_RadioState s_onStateRequest()
{
	RIL_RadioState rs = h_radiofunctions->onStateRequest();
	LOGD("s_onStateRequest(): %d\n", rs);
	return rs;
}

static int s_supports(int r)
{
	int i = h_radiofunctions->supports(r);
	LOGD("s_supports(%d): %d\n", r, i);
	return i;
}

static void s_onCancel(RIL_Token t)
{
	LOGD("s_onCancel(%p)\n", t);
	h_radiofunctions->onCancel(t);
}

static char smdbuf[8192], ptybuf[8192];

static void *s_readsmd(void *p)
{
	int count;
	do {
		count = read(s_fd, smdbuf, sizeof(smdbuf));
		if (count <= 0)
			continue;
		LOGD("<< %.*s", count, smdbuf);
		write(p_fd, smdbuf, count);
	} while(1);
	return NULL;
}

static void *s_readpty(void *p)
{
	int count;
	do {
		count = read(p_fd, ptybuf, sizeof(ptybuf));
		if (count <= 0)
			continue;
		LOGD(">> %.*s", count, ptybuf);
		write(s_fd, ptybuf, count);
	} while(1);
	return NULL;
}

const RIL_RadioFunctions *RIL_Init(const struct RIL_Env *env, int argc, char **argv) 
{
	RIL_RadioFunctions *(*h_RIL_Init)(const struct RIL_Env *env, int argc, char **argv);
	void *h_lib;
	int i;
	pthread_attr_t attr;
	pthread_t tid;
	pid_t pid;
	char *pt_name;
	struct termios ts;

	h_rilenv = env;
    LOGD("----RIL SPY v0.1, argc=%d----\n", argc);
	for (i=0; i<argc; i++)
		LOGD(" \"%s\"\n", argv[i]);

	h_lib=dlopen("/system/lib/libhtc_ril.so", 0);
	if (!h_lib)
		return NULL;
	h_RIL_Init = dlsym(h_lib, "RIL_Init");
	if (!h_RIL_Init)
		return NULL;

	p_fd = open("/dev/ptmx", O_RDWR);
	pt_name = ptsname(p_fd);
	grantpt(p_fd);
	unlockpt(p_fd);
	tcgetattr(p_fd, &ts);
	cfmakeraw(&ts);
	ts.c_cc[VMIN] = 1;
	ts.c_cc[VTIME] = 10;
	ts.c_cflag |= CS8;
	tcsetattr(p_fd, TCSANOW, &ts);

	pid = fork();
	if (pid == 0) {
		execl("/system/lib/rilspy_helper", "rilspy_helper", pt_name, NULL);
	} else {
		waitpid(pid, NULL, 0);
	}
	s_fd = open("/dev/smd00", O_RDWR);

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_create(&tid, &attr, s_readpty, NULL);
	pthread_create(&tid, &attr, s_readsmd, NULL);
	h_radiofunctions = h_RIL_Init(&s_rilenv, argc, argv);
	s_radiofunctions.version = h_radiofunctions->version;
	s_radiofunctions.getVersion = h_radiofunctions->getVersion;

	return &s_radiofunctions;
}
