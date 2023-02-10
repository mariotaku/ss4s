#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef enum SS4S_LogLevel {
    /**
      * Irrecoverable error, and the process should be aborted
      */
    SS4S_LogLevelFatal,
    /**
     * Error that should close player
     */
    SS4S_LogLevelError,
    /**
     * Problem that can be self-recovered
     */
    SS4S_LogLevelWarn,
    /**
     * Informative message
     */
    SS4S_LogLevelInfo,
    SS4S_LogLevelDebug,
    SS4S_LogLevelVerbose,
} SS4S_LogLevel;

typedef void(SS4S_LoggingFunction)(SS4S_LogLevel level, const char *tag, const char *fmt, ...)
        __attribute__ ((format (printf, 3, 4)));

#ifdef __cplusplus
}
#endif