
#ifndef LI_LOG_H
#define LI_LOG_H

#if defined(LOG_TAG)

#ifndef LOG_FD
#define LOG_FD stdout
#endif 


#define LOG_MSG(format, ... )	\
	fprintf(LOG_FD, format, ##__VA_ARGS__);

#define LOG_SEMI ":"
#define LOG_INFO "Info"
#define LOG_ERROR "Error"
#define LOG_WARNING "Warning"

#define LOGI(format, ...) \
	LOG_MSG(LOG_TAG LOG_SEMI LOG_INFO LOG_SEMI format, ##__VA_ARGS__)


#define LOGE(format, ...) \
	LOG_MSG(LOG_TAG LOG_SEMI LOG_ERROR LOG_SEMI format, ##__VA_ARGS__)


#define LOGW(format, ...) \
	LOG_MSG(LOG_TAG LOG_SEMI LOG_WARNING LOG_SEMI format, ##__VA_ARGS__)
#else

#define LOGI(format, ...) 
#define LOGE(format, ...) 
#define LOGW(format, ...) \

#endif 

#endif 


