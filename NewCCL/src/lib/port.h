#ifndef PORT_H
#define PORT_H

extern unsigned int port_rand(void);

#if 0
extern void port_message(char *buf, unsigned int bufsiz, const char *format, ...);

extern int port_timetostr(char *str, unsigned int len, long long int timestamp);
#endif

extern unsigned long long int port_current_time(void);

extern char *port_strncpy(char *dst, const char *src, unsigned int dsiz);

extern void *port_malloc(unsigned int size);

extern void port_free(void *ptr);

extern int port_strcmp(const char *str1, const char *str2);

extern char *port_strndup(const char *str, unsigned int maxsize);

extern int port_strncasecmp(const char *s1, const char *s2, unsigned int n);

extern unsigned int port_strnlen(const char *str, unsigned int maxsize);

extern int port_inttostr(char *str, unsigned int size, long long int number);

#endif
