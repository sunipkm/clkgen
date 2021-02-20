#include <stdint.h>
#include <unistd.h>
#include <string.h>
#ifndef __APPLE__
#include <sys/timerfd.h>
#else
#include <dispatch/dispatch.h>
#include <stdlib.h>
#endif
#include <pthread.h>
#include <poll.h>
#include <stdio.h>

#include "timer_gen.h"

#define MAX_TIMER_COUNT 1000

#ifndef __APPLE__
struct timer_node
{
    int fd;
    time_handler callback;
    void *user_data;
    unsigned long long int interval;
    t_timer type;
    struct timer_node *next;
};

static void *_timer_thread(void *data);
static pthread_t g_thread_id;
static struct timer_node *g_head = NULL;

#else
static dispatch_queue_t queue;
static dispatch_source_t *timers = NULL;
static size_t ntimers = 0;
static void sigtrap(int sig)
{
    for (size_t i = 0; i < ntimers; i++)
        dispatch_source_cancel(timers[i]);
}
#endif

int initialize()
{
#ifndef __APPLE__
    if (pthread_create(&g_thread_id, NULL, _timer_thread, NULL))
    {
        /*Thread creation failed*/
        return -1;
    }

    return 1;
#else
    signal(SIGINT, sigtrap);
    queue = dispatch_queue_create("timer_gen_timer_queue", 0);
    return 1;
#endif
}

size_t start_timer(unsigned long long int interval, time_handler handler, t_timer type, void *user_data)
{
#ifndef __APPLE__
    struct timer_node *new_node = NULL;
    struct itimerspec new_value;

    new_node = (struct timer_node *)malloc(sizeof(struct timer_node));

    if (new_node == NULL)
        return 0;

    new_node->callback = handler;
    new_node->user_data = user_data;
    new_node->interval = interval;
    new_node->type = type;

    new_node->fd = timerfd_create(CLOCK_REALTIME, 0);

    if (new_node->fd == -1)
    {
        free(new_node);
        return 0;
    }

    new_value.it_value.tv_sec = interval / 1000000000;
    new_value.it_value.tv_nsec = (interval % 1000000000);

    if (type == TIMER_PERIODIC)
    {
        new_value.it_interval.tv_sec = interval / 1000000000;
        new_value.it_interval.tv_nsec = (interval % 1000000000);
    }
    else
    {
        new_value.it_interval.tv_sec = 0;
        new_value.it_interval.tv_nsec = 0;
    }

    timerfd_settime(new_node->fd, 0, &new_value, NULL);

    /*Inserting the timer node into the list*/
    new_node->next = g_head;
    g_head = new_node;

    return (size_t)new_node;
#else
    ntimers++;
    if (timers == NULL)
        timers = (dispatch_source_t *)malloc(sizeof(dispatch_source_t));
    else
        timers = (dispatch_source_t *)realloc(timers, ntimers * sizeof(dispatch_source_t));
    timers[ntimers - 1] = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, queue);
    dispatch_source_set_event_handler(timers[ntimers - 1], ^{
      handler(ntimers - 1, user_data);
    });
    fflush(stdout);
    dispatch_source_set_cancel_handler(timers[ntimers - 1], ^{
      dispatch_release(timers[ntimers - 1]);
      dispatch_release(queue);
    });
    fflush(stdout);
    dispatch_time_t start = dispatch_time(DISPATCH_TIME_NOW, 0);
    dispatch_source_set_timer(timers[ntimers - 1], start, interval, 0);
    dispatch_resume(timers[ntimers - 1]);
    return ntimers;
#endif
}

void stop_timer(size_t timer_id)
{
#ifndef __APPLE__
    struct timer_node *tmp = NULL;
    struct timer_node *node = (struct timer_node *)timer_id;

    if (node == NULL)
        return;

    close(node->fd);

    if (node == g_head)
    {
        g_head = g_head->next;
    }
    else
    {

        tmp = g_head;

        while (tmp && tmp->next != node)
            tmp = tmp->next;

        if (tmp)
        {
            /*tmp->next can not be NULL here.*/
            tmp->next = tmp->next->next;
        }
    }
    if (node)
        free(node);
#else
    if (timer_id - 1 < ntimers)
        dispatch_source_cancel(timers[timer_id - 1]);
#endif
}

void finalize()
{
#ifndef __APPLE__
    while (g_head)
        stop_timer((size_t)g_head);

    pthread_cancel(g_thread_id);
    pthread_join(g_thread_id, NULL);
#else
    for (size_t i = 0; i < ntimers; i++)
        dispatch_source_cancel(timers[i]);
    free(timers);
#endif
}
#ifndef __APPLE__
struct timer_node *_get_timer_from_fd(int fd)
{
    struct timer_node *tmp = g_head;

    while (tmp)
    {
        if (tmp->fd == fd)
            return tmp;

        tmp = tmp->next;
    }
    return NULL;
}

void *_timer_thread(void *data)
{
    struct pollfd ufds[MAX_TIMER_COUNT] = {{0}};
    int iMaxCount = 0;
    struct timer_node *tmp = NULL;
    int read_fds = 0, i, s;
    uint64_t exp;

    while (1)
    {
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
        pthread_testcancel();
        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

        iMaxCount = 0;
        tmp = g_head;

        memset(ufds, 0, sizeof(struct pollfd) * MAX_TIMER_COUNT);
        while (tmp)
        {
            ufds[iMaxCount].fd = tmp->fd;
            ufds[iMaxCount].events = POLLIN;
            iMaxCount++;

            tmp = tmp->next;
        }
        read_fds = poll(ufds, iMaxCount, 100);

        if (read_fds <= 0)
            continue;

        for (i = 0; i < iMaxCount; i++)
        {
            if (ufds[i].revents & POLLIN)
            {
                s = read(ufds[i].fd, &exp, sizeof(uint64_t));

                if (s != sizeof(uint64_t))
                    continue;

                tmp = _get_timer_from_fd(ufds[i].fd);

                if (tmp && tmp->callback)
                    tmp->callback((size_t)tmp, tmp->user_data);
            }
        }
    }

    return NULL;
}
#endif