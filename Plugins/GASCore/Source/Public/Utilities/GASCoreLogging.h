// Copyright DermanDanisman, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

// Declare the log category
GASCORE_API DECLARE_LOG_CATEGORY_EXTERN(LogGASCore, Log, All);

// Log macros for GASCore
#define GASCORE_LOG(Verbosity, Format, ...) UE_LOG(LogGASCore, Verbosity, Format, ##__VA_ARGS__)

// Convenience macros for different verbosity levels
#define GASCORE_LOG_VERBOSE(Format, ...) GASCORE_LOG(Verbose, Format, ##__VA_ARGS__)
#define GASCORE_LOG_VERYVERBOSE(Format, ...) GASCORE_LOG(VeryVerbose, Format, ##__VA_ARGS__)
#define GASCORE_LOG_LOG(Format, ...) GASCORE_LOG(Log, Format, ##__VA_ARGS__)
#define GASCORE_LOG_WARNING(Format, ...) GASCORE_LOG(Warning, Format, ##__VA_ARGS__)
#define GASCORE_LOG_ERROR(Format, ...) GASCORE_LOG(Error, Format, ##__VA_ARGS__)
#define GASCORE_LOG_FATAL(Format, ...) GASCORE_LOG(Fatal, Format, ##__VA_ARGS__)

// Logging with condition
#define GASCORE_LOG_IF(Condition, Verbosity, Format, ...) \
if (Condition) { GASCORE_LOG(Verbosity, Format, ##__VA_ARGS__); }