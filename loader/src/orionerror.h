#pragma once

enum OrionError
{
	ORION_ERROR_NONE,
	ORION_ERROR_CREATE_SSL_CLIENT,
	ORION_ERROR_DOWNLOAD_MODULE_1,
	ORION_ERROR_DOWNLOAD_MODULE_2,
	ORION_ERROR_INVALID_MODULE_1,
	ORION_ERROR_INVALID_MODULE_2,
	ORION_ERROR_INVALID_MODULE_3,
	ORION_ERROR_INVALID_MODULE_4,
	ORION_ERROR_INVALID_MODULE_5,
	ORION_ERROR_INVALID_MODULE_6,
	ORION_ERROR_INVALID_MODULE_7,
	ORION_ERROR_INVALID_MODULE_8,
	ORION_ERROR_INVALID_MODULE_9,
	ORION_ERROR_ALLOCATE_MEMORY_1,
	ORION_ERROR_ALLOCATE_MEMORY_2,
	ORION_ERROR_ALLOCATE_MEMORY_3,
	ORION_ERROR_ALLOCATE_MEMORY_4,
	ORION_ERROR_ALLOCATE_MEMORY_5,
	ORION_ERROR_ALLOCATE_MEMORY_6,
	ORION_ERROR_ALLOCATE_MEMORY_7,
	ORION_ERROR_ALLOCATE_MEMORY_8,
	ORION_ERROR_ALLOCATE_MEMORY_9,
	ORION_ERROR_ALLOCATE_MEMORY_10,
	ORION_ERROR_ALLOCATE_MEMORY_11,
	ORION_ERROR_ALLOCATE_MEMORY_12,
	ORION_ERROR_ALLOCATE_MEMORY_13,
	ORION_ERROR_ALLOCATE_MEMORY_14,
	ORION_ERROR_ALLOCATE_MEMORY_15,
	ORION_ERROR_ALLOCATE_MEMORY_16,
	ORION_ERROR_ALLOCATE_MEMORY_17,
	ORION_ERROR_ALLOCATE_MEMORY_18,
	ORION_ERROR_ALLOCATE_MEMORY_19,
	ORION_ERROR_ALLOCATE_MEMORY_20,
	ORION_ERROR_ALLOCATE_MEMORY_21,
	ORION_ERROR_ALLOCATE_MEMORY_22,
	ORION_ERROR_ALLOCATE_MEMORY_23,
	ORION_ERROR_ALLOCATE_MEMORY_24,
	ORION_ERROR_ALLOCATE_MEMORY_25,
	ORION_ERROR_ALLOCATE_MEMORY_26,
	ORION_ERROR_ALLOCATE_MEMORY_27,
	ORION_ERROR_ALLOCATE_MEMORY_28,
	ORION_ERROR_ALLOCATE_MEMORY_29,
	ORION_ERROR_ALLOCATE_MEMORY_30,
	ORION_ERROR_REALLOCATE_MEMORY_1,
	ORION_ERROR_REALLOCATE_MEMORY_2,
	ORION_ERROR_REALLOCATE_MEMORY_3,
	ORION_ERROR_REALLOCATE_MEMORY_4,
	ORION_ERROR_REALLOCATE_MEMORY_5,
	ORION_ERROR_REALLOCATE_MEMORY_6,
	ORION_ERROR_REALLOCATE_MEMORY_7,
	ORION_ERROR_REALLOCATE_MEMORY_8,
	ORION_ERROR_REALLOCATE_MEMORY_9,
	ORION_ERROR_REALLOCATE_MEMORY_10,
	ORION_ERROR_REALLOCATE_MEMORY_11,
	ORION_ERROR_FREE_MEMORY_1,
	ORION_ERROR_FREE_MEMORY_2,
	ORION_ERROR_FREE_MEMORY_3,
	ORION_ERROR_FREE_MEMORY_4,
	ORION_ERROR_FREE_MEMORY_5,
	ORION_ERROR_FREE_MEMORY_6,
	ORION_ERROR_FREE_MEMORY_7,
	ORION_ERROR_FREE_MEMORY_8,
	ORION_ERROR_FREE_MEMORY_9,
	ORION_ERROR_FREE_MEMORY_10,
	ORION_ERROR_FREE_MEMORY_11,
	ORION_ERROR_FREE_MEMORY_12,
	ORION_ERROR_FREE_MEMORY_13,
	ORION_ERROR_FREE_MEMORY_14,
	ORION_ERROR_FREE_MEMORY_15,
	ORION_ERROR_FREE_MEMORY_16,
	ORION_ERROR_FREE_MEMORY_17,
	ORION_ERROR_FREE_MEMORY_18,
	ORION_ERROR_FREE_MEMORY_19,
	ORION_ERROR_FREE_MEMORY_20,
	ORION_ERROR_FREE_MEMORY_21,
	ORION_ERROR_FREE_MEMORY_22,
	ORION_ERROR_FREE_MEMORY_23,
	ORION_ERROR_FREE_MEMORY_24,
	ORION_ERROR_FREE_MEMORY_25,
	ORION_ERROR_FREE_MEMORY_26,
	ORION_ERROR_FREE_MEMORY_27,
	ORION_ERROR_FREE_MEMORY_28,
	ORION_ERROR_FREE_MEMORY_29,
	ORION_ERROR_FREE_MEMORY_30,
	ORION_ERROR_FREE_MEMORY_31,
	ORION_ERROR_FREE_MEMORY_32,
	ORION_ERROR_FREE_MEMORY_33,
	ORION_ERROR_FREE_MEMORY_34,
	ORION_ERROR_FREE_MEMORY_35,
	ORION_ERROR_FREE_MEMORY_36,
	ORION_ERROR_FREE_MEMORY_37,
	ORION_ERROR_LOAD_LIBRARY_1,
	ORION_ERROR_LOAD_LIBRARY_2,
	ORION_ERROR_LOAD_LIBRARY_3,
	ORION_ERROR_LOAD_LIBRARY_4,
	ORION_ERROR_FREE_LIBRARY_1,
	ORION_ERROR_FREE_LIBRARY_2,
	ORION_ERROR_FREE_LIBRARY_3,
	ORION_ERROR_FREE_LIBRARY_4,
	ORION_ERROR_FREE_LIBRARY_5,
	ORION_ERROR_GET_FUNCTION_ADDRESS_1,
	ORION_ERROR_GET_FUNCTION_ADDRESS_2,
	ORION_ERROR_GET_FUNCTION_ADDRESS_3,
	ORION_ERROR_GET_FUNCTION_ADDRESS_4,
	ORION_ERROR_GET_FUNCTION_ADDRESS_5,
	ORION_ERROR_GET_FUNCTION_ADDRESS_6,
	ORION_ERROR_GET_FUNCTION_ADDRESS_7,
	ORION_ERROR_GET_FUNCTION_ADDRESS_8,
	ORION_ERROR_GET_FUNCTION_ADDRESS_9,
	ORION_ERROR_SETUP_EXCEPTIONS,
	ORION_ERROR_START_MODULE,
	ORION_ERROR_STOP_MODULE,
	ORION_ERROR_INVALID_DATA,
	ORION_ERROR_QUERY_SYSTEM_INFORMATION_1,
	ORION_ERROR_QUERY_SYSTEM_INFORMATION_2,
	ORION_ERROR_QUERY_SYSTEM_INFORMATION_3,
	ORION_ERROR_QUERY_SYSTEM_INFORMATION_4,
	ORION_ERROR_QUERY_SYSTEM_INFORMATION_5,
	ORION_ERROR_QUERY_SYSTEM_INFORMATION_6,
	ORION_ERROR_QUERY_SYSTEM_INFORMATION_7,
	ORION_ERROR_QUERY_SYSTEM_INFORMATION_8,
	ORION_ERROR_QUERY_SYSTEM_INFORMATION_9,
	ORION_ERROR_QUERY_SYSTEM_INFORMATION_10,
	ORION_ERROR_QUERY_SYSTEM_INFORMATION_11,
	ORION_ERROR_SYSTEM_INTEGRITY_VIOLATION_1,
	ORION_ERROR_SYSTEM_INTEGRITY_VIOLATION_2,
	ORION_ERROR_SYSTEM_INTEGRITY_VIOLATION_3,
	ORION_ERROR_CREATE_THREAD_1,
	ORION_ERROR_CREATE_THREAD_2,
	ORION_ERROR_CREATE_THREAD_3,
	ORION_ERROR_WAIT_FOR_SINGLE_OBJECT_1,
	ORION_ERROR_WAIT_FOR_SINGLE_OBJECT_2,
	ORION_ERROR_WAIT_FOR_SINGLE_OBJECT_3,
	ORION_ERROR_CLOSE_HANDLE_1,
	ORION_ERROR_CLOSE_HANDLE_2,
	ORION_ERROR_CLOSE_HANDLE_3,
	ORION_ERROR_CLOSE_HANDLE_4,
	ORION_ERROR_CLOSE_HANDLE_5,
	ORION_ERROR_CLOSE_HANDLE_6,
	ORION_ERROR_CLOSE_HANDLE_7,
	ORION_ERROR_CLOSE_HANDLE_8,
	ORION_ERROR_CLOSE_HANDLE_9,
	ORION_ERROR_CLOSE_HANDLE_10,
	ORION_ERROR_CLOSE_HANDLE_11,
	ORION_ERROR_CLOSE_HANDLE_12,
	ORION_ERROR_OPEN_PROCESS_TOKEN_1,
	ORION_ERROR_OPEN_PROCESS_TOKEN_2,
	ORION_ERROR_LOOKUP_PRIVILEGE_VALUE_1,
	ORION_ERROR_LOOKUP_PRIVILEGE_VALUE_2,
	ORION_ERROR_ADJUST_PRIVILEGES_TOKEN_1,
	ORION_ERROR_ADJUST_PRIVILEGES_TOKEN_2,
	ORION_ERROR_STARTUP_WSA,
	ORION_ERROR_GET_HOST_BY_NAME,
	ORION_ERROR_CREATE_SOCKET,
	ORION_ERROR_CONNECT_TO_SERVER,
	ORION_ERROR_SET_SOCKET_OPTION_1,
	ORION_ERROR_SET_SOCKET_OPTION_2,
	ORION_ERROR_SET_SOCKET_OPTION_3,
	ORION_ERROR_INITIALIZE_COM_LIBRARY,
	ORION_ERROR_CREATE_INSTANCE,
	ORION_ERROR_CONNECT_TO_WMI_SERVER,
	ORION_ERROR_SET_PROXY_BLANKET,
	ORION_ERROR_INVALID_TOKEN,
	ORION_ERROR_INVALID_KEY,
	ORION_ERROR_GET_CPU_INFORMATION,
	ORION_ERROR_SEND_DATA_1,
	ORION_ERROR_SEND_DATA_2,
	ORION_ERROR_SEND_DATA_3,
	ORION_ERROR_SEND_DATA_4,
	ORION_ERROR_SEND_DATA_5,
	ORION_ERROR_SEND_DATA_6,
	ORION_ERROR_SEND_DATA_7,
	ORION_ERROR_SEND_DATA_8,
	ORION_ERROR_SEND_DATA_9,
	ORION_ERROR_SEND_DATA_10,
	ORION_ERROR_SEND_DATA_11,
	ORION_ERROR_SEND_DATA_12,
	ORION_ERROR_SEND_DATA_13,
	ORION_ERROR_SEND_DATA_14,
	ORION_ERROR_SEND_DATA_15,
	ORION_ERROR_RECEIVE_DATA_1,
	ORION_ERROR_RECEIVE_DATA_2,
	ORION_ERROR_RECEIVE_DATA_3,
	ORION_ERROR_RECEIVE_DATA_4,
	ORION_ERROR_RECEIVE_DATA_5,
	ORION_ERROR_RECEIVE_DATA_6,
	ORION_ERROR_RECEIVE_DATA_7,
	ORION_ERROR_RECEIVE_DATA_8,
	ORION_ERROR_RECEIVE_DATA_9,
	ORION_ERROR_RECEIVE_DATA_10,
	ORION_ERROR_RECEIVE_DATA_11,
	ORION_ERROR_RECEIVE_DATA_12,
	ORION_ERROR_RECEIVE_DATA_13,
	ORION_ERROR_RECEIVE_DATA_14,
	ORION_ERROR_RECEIVE_DATA_15,
	ORION_ERROR_RECEIVE_DATA_16,
	ORION_ERROR_RECEIVE_DATA_17,
	ORION_ERROR_RECEIVE_DATA_18,
	ORION_ERROR_RECEIVE_DATA_19,
	ORION_ERROR_RECEIVE_DATA_20,
	ORION_ERROR_RECEIVE_DATA_21,
	ORION_ERROR_RECEIVE_DATA_22,
	ORION_ERROR_RECEIVE_DATA_23,
	ORION_ERROR_RECEIVE_DATA_24,
	ORION_ERROR_RECEIVE_DATA_25,
	ORION_ERROR_RECEIVE_DATA_26,
	ORION_ERROR_RECEIVE_DATA_27,
	ORION_ERROR_RECEIVE_DATA_28,
	ORION_ERROR_EXPIRED_KEY,
	ORION_ERROR_FROZEN_KEY,
	ORION_ERROR_BANNED_KEY,
	ORION_ERROR_BRANDING_FORMAT_STRING,
	ORION_ERROR_OPEN_REGISTRY_KEY_1,
	ORION_ERROR_OPEN_REGISTRY_KEY_2,
	ORION_ERROR_OPEN_REGISTRY_KEY_3,
	ORION_ERROR_OPEN_REGISTRY_KEY_4,
	ORION_ERROR_CLOSE_REGISTRY_KEY_1,
	ORION_ERROR_CLOSE_REGISTRY_KEY_2,
	ORION_ERROR_CLOSE_REGISTRY_KEY_3,
	ORION_ERROR_CLOSE_REGISTRY_KEY_4,
	ORION_ERROR_QUERY_REGISTRY_VALUE_1,
	ORION_ERROR_QUERY_REGISTRY_VALUE_2,
	ORION_ERROR_QUERY_REGISTRY_VALUE_3,
	ORION_ERROR_QUERY_REGISTRY_VALUE_4,
	ORION_ERROR_QUERY_REGISTRY_VALUE_5,
	ORION_ERROR_QUERY_REGISTRY_VALUE_6,
	ORION_ERROR_QUERY_REGISTRY_VALUE_7,
	ORION_ERROR_QUERY_REGISTRY_VALUE_8,
	ORION_ERROR_QUERY_REGISTRY_VALUE_9,
	ORION_ERROR_QUERY_REGISTRY_VALUE_10,
	ORION_ERROR_QUERY_REGISTRY_VALUE_11,
	ORION_ERROR_QUERY_REGISTRY_VALUE_12,
	ORION_ERROR_EXECUTE_QUERY,
	ORION_ERROR_CLEAR_VARIANT_1,
	ORION_ERROR_CLEAR_VARIANT_2,
	ORION_ERROR_CLEAR_VARIANT_3,
	ORION_ERROR_INITIALIZE_PARSER_1,
	ORION_ERROR_INITIALIZE_PARSER_2,
	ORION_ERROR_INITIALIZE_PARSER_3,
	ORION_ERROR_DEVICE_IO_CONTROL_FILE,
	ORION_ERROR_GET_DEVICE_COUNT,
	ORION_ERROR_GET_DEVICE_HANDLE_BY_INDEX,
	ORION_ERROR_GET_DEVICE_UUID,
	ORION_ERROR_SHUTDOWN,
	ORION_ERROR_GET_ADAPTERS_INFORMATION_1,
	ORION_ERROR_GET_ADAPTERS_INFORMATION_2,
	ORION_ERROR_CHANGED_SYSTEM,
	ORION_ERROR_BANNED_SYSTEM,
	ORION_ERROR_INVALID_SYSTEM,
	ORION_ERROR_ENABLED_SECURE_BOOT,
	ORION_ERROR_DISABLED_VIRTUALIZATION,
	ORION_ERROR_DISABLED_HYPER_V,
	ORION_ERROR_OPEN_FILE_1,
	ORION_ERROR_OPEN_FILE_2,
	ORION_ERROR_SET_INFORMATION_FILE_1,
	ORION_ERROR_SET_INFORMATION_FILE_2,
	ORION_ERROR_CREATE_FILE,
	ORION_ERROR_WRITE_FILE,
	ORION_ERROR_REBOOT_SYSTEM,
	ORION_ERROR_CREATE_PROCESS_1,
	ORION_ERROR_CREATE_PROCESS_2,
	ORION_ERROR_CREATE_PROCESS_3,
	ORION_ERROR_DELAY_EXECUTION,
	ORION_ERROR_INVALID_ALLOCATE_ADDRESS_1,
	ORION_ERROR_INVALID_ALLOCATE_ADDRESS_2,
	ORION_ERROR_INVALID_LIBRARY_1,
	ORION_ERROR_INVALID_LIBRARY_2,
	ORION_ERROR_INVALID_LIBRARY_3,
	ORION_ERROR_INVALID_LIBRARY_4,
	ORION_ERROR_INVALID_LIBRARY_5,
	ORION_ERROR_INVALID_LIBRARY_6,
	ORION_ERROR_INVALID_LIBRARY_7,
	ORION_ERROR_INVALID_LIBRARY_8,
	ORION_ERROR_INVALID_LIBRARY_9,
	ORION_ERROR_INVALID_LIBRARY_10,
	ORION_ERROR_INVALID_LIBRARY_11,
	ORION_ERROR_INVALID_LIBRARY_12,
	ORION_ERROR_READ_MEMORY_1,
	ORION_ERROR_READ_MEMORY_2,
	ORION_ERROR_READ_MEMORY_3,
	ORION_ERROR_READ_MEMORY_4,
	ORION_ERROR_FIND_LIBRARY,
	ORION_ERROR_GET_CONTEXT_THREAD,
	ORION_ERROR_SET_CONTEXT_THREAD,
	ORION_ERROR_RESUME_THREAD,
	ORION_ERROR_FLUSH_DNS_RESOLVER_CACHE,
	ORION_ERROR_INITIALIZE_DISM,
	ORION_ERROR_OPEN_DISM_SESSION,
	ORION_ERROR_GET_DISM_FEATURE_INFORMATION,
	ORION_ERROR_DELETE_DISM,
	ORION_ERROR_CLOSE_DISM_SESSION,
	ORION_ERROR_SHUTDOWN_DISM,
	ORION_ERROR_GET_THREAD_GROUP_AFFINITY,
	ORION_ERROR_SET_THREAD_GROUP_AFFINITY_1,
	ORION_ERROR_SET_THREAD_GROUP_AFFINITY_2,
	ORION_ERROR_GET_ACTIVE_PROCESSOR_GROUP_COUNT,
	ORION_ERROR_GET_ACTIVE_PROCESSOR_COUNT,
	ORION_ERROR_INITIALIZE_HYPERVISOR,
	ORION_ERROR_NOT_FLASH_DRIVE,
	ORION_ERROR_INVALID_FIRMWARE_TYPE,
	ORION_ERROR_MAX
};

// Returns a human-readable string for a given OrionError code.
// Used by the launcher to display error messages.
inline const char* OrionErrorToString(OrionError err)
{
	switch (err)
	{
	case ORION_ERROR_NONE:                          return "No error";
	case ORION_ERROR_CREATE_SSL_CLIENT:             return "Failed to create SSL client";
	case ORION_ERROR_DOWNLOAD_MODULE_1:             return "Download module error (1)";
	case ORION_ERROR_DOWNLOAD_MODULE_2:             return "Download module error (2)";
	case ORION_ERROR_INVALID_MODULE_1:              return "Invalid module: bad DOS signature";
	case ORION_ERROR_INVALID_MODULE_2:              return "Invalid module: bad NT signature";
	case ORION_ERROR_INVALID_MODULE_3:              return "Invalid module: not x64 binary";
	case ORION_ERROR_INVALID_MODULE_4:              return "Invalid module: not PE32+ optional header";
	case ORION_ERROR_INVALID_MODULE_5:              return "Invalid module: no sections";
	case ORION_ERROR_INVALID_MODULE_6:              return "Invalid module: no import directory";
	case ORION_ERROR_INVALID_MODULE_7:              return "Invalid module: no relocation directory";
	case ORION_ERROR_INVALID_MODULE_8:              return "Invalid module: no exception directory";
	case ORION_ERROR_INVALID_MODULE_9:              return "Invalid module error (9)";
	case ORION_ERROR_ALLOCATE_MEMORY_1:             return "Memory allocation failed (1)";
	case ORION_ERROR_ALLOCATE_MEMORY_2:             return "Memory allocation failed (2)";
	case ORION_ERROR_ALLOCATE_MEMORY_3:             return "Memory allocation failed (3)";
	case ORION_ERROR_ALLOCATE_MEMORY_4:             return "Memory allocation failed (4)";
	case ORION_ERROR_ALLOCATE_MEMORY_5:             return "Memory allocation failed (5)";
	case ORION_ERROR_ALLOCATE_MEMORY_6:             return "Memory allocation failed (6)";
	case ORION_ERROR_ALLOCATE_MEMORY_7:             return "Memory allocation failed (7)";
	case ORION_ERROR_ALLOCATE_MEMORY_8:             return "Memory allocation failed (8)";
	case ORION_ERROR_ALLOCATE_MEMORY_9:             return "Memory allocation failed (9)";
	case ORION_ERROR_ALLOCATE_MEMORY_10:            return "Memory allocation failed (10)";
	case ORION_ERROR_ALLOCATE_MEMORY_11:            return "Memory allocation failed (11)";
	case ORION_ERROR_ALLOCATE_MEMORY_12:            return "Memory allocation failed (12)";
	case ORION_ERROR_ALLOCATE_MEMORY_13:            return "Memory allocation failed (13)";
	case ORION_ERROR_ALLOCATE_MEMORY_14:            return "Memory allocation failed (14)";
	case ORION_ERROR_ALLOCATE_MEMORY_15:            return "Memory allocation failed (15)";
	case ORION_ERROR_ALLOCATE_MEMORY_16:            return "Memory allocation failed (16)";
	case ORION_ERROR_ALLOCATE_MEMORY_17:            return "Memory allocation failed (17)";
	case ORION_ERROR_ALLOCATE_MEMORY_18:            return "Memory allocation failed (18)";
	case ORION_ERROR_ALLOCATE_MEMORY_19:            return "Memory allocation failed (19)";
	case ORION_ERROR_ALLOCATE_MEMORY_20:            return "Memory allocation failed (20)";
	case ORION_ERROR_ALLOCATE_MEMORY_21:            return "Memory allocation failed (21)";
	case ORION_ERROR_ALLOCATE_MEMORY_22:            return "Memory allocation failed (22)";
	case ORION_ERROR_ALLOCATE_MEMORY_23:            return "Memory allocation failed (23)";
	case ORION_ERROR_ALLOCATE_MEMORY_24:            return "Memory allocation failed (24)";
	case ORION_ERROR_ALLOCATE_MEMORY_25:            return "Memory allocation failed (25)";
	case ORION_ERROR_ALLOCATE_MEMORY_26:            return "Memory allocation failed (26)";
	case ORION_ERROR_ALLOCATE_MEMORY_27:            return "Memory allocation failed (27)";
	case ORION_ERROR_ALLOCATE_MEMORY_28:            return "Memory allocation failed (28)";
	case ORION_ERROR_ALLOCATE_MEMORY_29:            return "Memory allocation failed (29)";
	case ORION_ERROR_ALLOCATE_MEMORY_30:            return "Memory allocation failed (30)";
	case ORION_ERROR_REALLOCATE_MEMORY_1:           return "Memory reallocation failed (1)";
	case ORION_ERROR_REALLOCATE_MEMORY_2:           return "Memory reallocation failed (2)";
	case ORION_ERROR_REALLOCATE_MEMORY_3:           return "Memory reallocation failed (3)";
	case ORION_ERROR_REALLOCATE_MEMORY_4:           return "Memory reallocation failed (4)";
	case ORION_ERROR_REALLOCATE_MEMORY_5:           return "Memory reallocation failed (5)";
	case ORION_ERROR_REALLOCATE_MEMORY_6:           return "Memory reallocation failed (6)";
	case ORION_ERROR_REALLOCATE_MEMORY_7:           return "Memory reallocation failed (7)";
	case ORION_ERROR_REALLOCATE_MEMORY_8:           return "Memory reallocation failed (8)";
	case ORION_ERROR_REALLOCATE_MEMORY_9:           return "Memory reallocation failed (9)";
	case ORION_ERROR_REALLOCATE_MEMORY_10:          return "Memory reallocation failed (10)";
	case ORION_ERROR_REALLOCATE_MEMORY_11:          return "Memory reallocation failed (11)";
	case ORION_ERROR_FREE_MEMORY_1:                 return "Memory free failed (1)";
	case ORION_ERROR_FREE_MEMORY_2:                 return "Memory free failed (2)";
	case ORION_ERROR_FREE_MEMORY_3:                 return "Memory free failed (3)";
	case ORION_ERROR_FREE_MEMORY_4:                 return "Memory free failed (4)";
	case ORION_ERROR_FREE_MEMORY_5:                 return "Memory free failed (5)";
	case ORION_ERROR_FREE_MEMORY_6:                 return "Memory free failed (6)";
	case ORION_ERROR_FREE_MEMORY_7:                 return "Memory free failed (7)";
	case ORION_ERROR_FREE_MEMORY_8:                 return "Memory free failed (8)";
	case ORION_ERROR_FREE_MEMORY_9:                 return "Memory free failed (9)";
	case ORION_ERROR_FREE_MEMORY_10:                return "Memory free failed (10)";
	case ORION_ERROR_FREE_MEMORY_11:                return "Memory free failed (11)";
	case ORION_ERROR_FREE_MEMORY_12:                return "Memory free failed (12)";
	case ORION_ERROR_FREE_MEMORY_13:                return "Memory free failed (13)";
	case ORION_ERROR_FREE_MEMORY_14:                return "Memory free failed (14)";
	case ORION_ERROR_FREE_MEMORY_15:                return "Memory free failed (15)";
	case ORION_ERROR_FREE_MEMORY_16:                return "Memory free failed (16)";
	case ORION_ERROR_FREE_MEMORY_17:                return "Memory free failed (17)";
	case ORION_ERROR_FREE_MEMORY_18:                return "Memory free failed (18)";
	case ORION_ERROR_FREE_MEMORY_19:                return "Memory free failed (19)";
	case ORION_ERROR_FREE_MEMORY_20:                return "Memory free failed (20)";
	case ORION_ERROR_FREE_MEMORY_21:                return "Memory free failed (21)";
	case ORION_ERROR_FREE_MEMORY_22:                return "Memory free failed (22)";
	case ORION_ERROR_FREE_MEMORY_23:                return "Memory free failed (23)";
	case ORION_ERROR_FREE_MEMORY_24:                return "Memory free failed (24)";
	case ORION_ERROR_FREE_MEMORY_25:                return "Memory free failed (25)";
	case ORION_ERROR_FREE_MEMORY_26:                return "Memory free failed (26)";
	case ORION_ERROR_FREE_MEMORY_27:                return "Memory free failed (27)";
	case ORION_ERROR_FREE_MEMORY_28:                return "Memory free failed (28)";
	case ORION_ERROR_FREE_MEMORY_29:                return "Memory free failed (29)";
	case ORION_ERROR_FREE_MEMORY_30:                return "Memory free failed (30)";
	case ORION_ERROR_FREE_MEMORY_31:                return "Memory free failed (31)";
	case ORION_ERROR_FREE_MEMORY_32:                return "Memory free failed (32)";
	case ORION_ERROR_FREE_MEMORY_33:                return "Memory free failed (33)";
	case ORION_ERROR_FREE_MEMORY_34:                return "Memory free failed (34)";
	case ORION_ERROR_FREE_MEMORY_35:                return "Memory free failed (35)";
	case ORION_ERROR_FREE_MEMORY_36:                return "Memory free failed (36)";
	case ORION_ERROR_FREE_MEMORY_37:                return "Memory free failed (37)";
	case ORION_ERROR_LOAD_LIBRARY_1:               return "LoadLibrary failed (1)";
	case ORION_ERROR_LOAD_LIBRARY_2:               return "LoadLibrary failed (2)";
	case ORION_ERROR_LOAD_LIBRARY_3:               return "LoadLibrary failed (3)";
	case ORION_ERROR_LOAD_LIBRARY_4:               return "LoadLibrary failed (4)";
	case ORION_ERROR_FREE_LIBRARY_1:               return "FreeLibrary failed (1)";
	case ORION_ERROR_FREE_LIBRARY_2:               return "FreeLibrary failed (2)";
	case ORION_ERROR_FREE_LIBRARY_3:               return "FreeLibrary failed (3)";
	case ORION_ERROR_FREE_LIBRARY_4:               return "FreeLibrary failed (4)";
	case ORION_ERROR_FREE_LIBRARY_5:               return "FreeLibrary failed (5)";
	case ORION_ERROR_GET_FUNCTION_ADDRESS_1:        return "GetProcAddress failed (1)";
	case ORION_ERROR_GET_FUNCTION_ADDRESS_2:        return "GetProcAddress failed (2)";
	case ORION_ERROR_GET_FUNCTION_ADDRESS_3:        return "GetProcAddress failed (3)";
	case ORION_ERROR_GET_FUNCTION_ADDRESS_4:        return "GetProcAddress failed (4)";
	case ORION_ERROR_GET_FUNCTION_ADDRESS_5:        return "GetProcAddress failed (5)";
	case ORION_ERROR_GET_FUNCTION_ADDRESS_6:        return "GetProcAddress failed (6)";
	case ORION_ERROR_GET_FUNCTION_ADDRESS_7:        return "GetProcAddress failed (7)";
	case ORION_ERROR_GET_FUNCTION_ADDRESS_8:        return "GetProcAddress failed (8)";
	case ORION_ERROR_GET_FUNCTION_ADDRESS_9:        return "GetProcAddress failed (9)";
	case ORION_ERROR_SETUP_EXCEPTIONS:              return "Failed to register exception table (RtlAddFunctionTable)";
	case ORION_ERROR_START_MODULE:                  return "DllMain DLL_PROCESS_ATTACH returned FALSE";
	case ORION_ERROR_STOP_MODULE:                   return "DllMain DLL_PROCESS_DETACH returned FALSE";
	case ORION_ERROR_INVALID_DATA:                  return "Invalid data";
	case ORION_ERROR_QUERY_SYSTEM_INFORMATION_1:    return "NtQuerySystemInformation failed (1)";
	case ORION_ERROR_QUERY_SYSTEM_INFORMATION_2:    return "NtQuerySystemInformation failed (2)";
	case ORION_ERROR_QUERY_SYSTEM_INFORMATION_3:    return "NtQuerySystemInformation failed (3)";
	case ORION_ERROR_QUERY_SYSTEM_INFORMATION_4:    return "NtQuerySystemInformation failed (4)";
	case ORION_ERROR_QUERY_SYSTEM_INFORMATION_5:    return "NtQuerySystemInformation failed (5)";
	case ORION_ERROR_QUERY_SYSTEM_INFORMATION_6:    return "NtQuerySystemInformation failed (6)";
	case ORION_ERROR_QUERY_SYSTEM_INFORMATION_7:    return "NtQuerySystemInformation failed (7)";
	case ORION_ERROR_QUERY_SYSTEM_INFORMATION_8:    return "NtQuerySystemInformation failed (8)";
	case ORION_ERROR_QUERY_SYSTEM_INFORMATION_9:    return "NtQuerySystemInformation failed (9)";
	case ORION_ERROR_QUERY_SYSTEM_INFORMATION_10:   return "NtQuerySystemInformation failed (10)";
	case ORION_ERROR_QUERY_SYSTEM_INFORMATION_11:   return "NtQuerySystemInformation failed (11)";
	case ORION_ERROR_SYSTEM_INTEGRITY_VIOLATION_1:  return "System integrity violation (1)";
	case ORION_ERROR_SYSTEM_INTEGRITY_VIOLATION_2:  return "System integrity violation (2)";
	case ORION_ERROR_SYSTEM_INTEGRITY_VIOLATION_3:  return "System integrity violation (3)";
	case ORION_ERROR_CREATE_THREAD_1:               return "CreateThread failed (1)";
	case ORION_ERROR_CREATE_THREAD_2:               return "CreateThread failed (2)";
	case ORION_ERROR_CREATE_THREAD_3:               return "CreateThread failed (3)";
	case ORION_ERROR_WAIT_FOR_SINGLE_OBJECT_1:      return "WaitForSingleObject failed (1)";
	case ORION_ERROR_WAIT_FOR_SINGLE_OBJECT_2:      return "WaitForSingleObject failed (2)";
	case ORION_ERROR_WAIT_FOR_SINGLE_OBJECT_3:      return "WaitForSingleObject failed (3)";
	case ORION_ERROR_CLOSE_HANDLE_1:                return "CloseHandle failed (1)";
	case ORION_ERROR_CLOSE_HANDLE_2:                return "CloseHandle failed (2)";
	case ORION_ERROR_CLOSE_HANDLE_3:                return "CloseHandle failed (3)";
	case ORION_ERROR_CLOSE_HANDLE_4:                return "CloseHandle failed (4)";
	case ORION_ERROR_CLOSE_HANDLE_5:                return "CloseHandle failed (5)";
	case ORION_ERROR_CLOSE_HANDLE_6:                return "CloseHandle failed (6)";
	case ORION_ERROR_CLOSE_HANDLE_7:                return "CloseHandle failed (7)";
	case ORION_ERROR_CLOSE_HANDLE_8:                return "CloseHandle failed (8)";
	case ORION_ERROR_CLOSE_HANDLE_9:                return "CloseHandle failed (9)";
	case ORION_ERROR_CLOSE_HANDLE_10:               return "CloseHandle failed (10)";
	case ORION_ERROR_CLOSE_HANDLE_11:               return "CloseHandle failed (11)";
	case ORION_ERROR_CLOSE_HANDLE_12:               return "CloseHandle failed (12)";
	case ORION_ERROR_OPEN_PROCESS_TOKEN_1:          return "OpenProcessToken failed (1)";
	case ORION_ERROR_OPEN_PROCESS_TOKEN_2:          return "OpenProcessToken failed (2)";
	case ORION_ERROR_LOOKUP_PRIVILEGE_VALUE_1:      return "LookupPrivilegeValue failed (1)";
	case ORION_ERROR_LOOKUP_PRIVILEGE_VALUE_2:      return "LookupPrivilegeValue failed (2)";
	case ORION_ERROR_ADJUST_PRIVILEGES_TOKEN_1:     return "AdjustTokenPrivileges failed (1)";
	case ORION_ERROR_ADJUST_PRIVILEGES_TOKEN_2:     return "AdjustTokenPrivileges failed (2)";
	case ORION_ERROR_STARTUP_WSA:                   return "WSAStartup failed";
	case ORION_ERROR_GET_HOST_BY_NAME:              return "gethostbyname failed";
	case ORION_ERROR_CREATE_SOCKET:                 return "Failed to create socket";
	case ORION_ERROR_CONNECT_TO_SERVER:             return "Failed to connect to server";
	case ORION_ERROR_SET_SOCKET_OPTION_1:           return "setsockopt failed (1)";
	case ORION_ERROR_SET_SOCKET_OPTION_2:           return "setsockopt failed (2)";
	case ORION_ERROR_SET_SOCKET_OPTION_3:           return "setsockopt failed (3)";
	case ORION_ERROR_INITIALIZE_COM_LIBRARY:        return "CoInitializeEx failed";
	case ORION_ERROR_CREATE_INSTANCE:               return "CoCreateInstance failed";
	case ORION_ERROR_CONNECT_TO_WMI_SERVER:         return "Failed to connect to WMI server";
	case ORION_ERROR_SET_PROXY_BLANKET:             return "CoSetProxyBlanket failed";
	case ORION_ERROR_INVALID_TOKEN:                 return "Invalid session token";
	case ORION_ERROR_INVALID_KEY:                   return "Invalid license key";
	case ORION_ERROR_GET_CPU_INFORMATION:           return "Failed to retrieve CPU information";
	case ORION_ERROR_SEND_DATA_1:                   return "Send data failed (1)";
	case ORION_ERROR_SEND_DATA_2:                   return "Send data failed (2)";
	case ORION_ERROR_SEND_DATA_3:                   return "Send data failed (3)";
	case ORION_ERROR_SEND_DATA_4:                   return "Send data failed (4)";
	case ORION_ERROR_SEND_DATA_5:                   return "Send data failed (5)";
	case ORION_ERROR_SEND_DATA_6:                   return "Send data failed (6)";
	case ORION_ERROR_SEND_DATA_7:                   return "Send data failed (7)";
	case ORION_ERROR_SEND_DATA_8:                   return "Send data failed (8)";
	case ORION_ERROR_SEND_DATA_9:                   return "Send data failed (9)";
	case ORION_ERROR_SEND_DATA_10:                  return "Send data failed (10)";
	case ORION_ERROR_SEND_DATA_11:                  return "Send data failed (11)";
	case ORION_ERROR_SEND_DATA_12:                  return "Send data failed (12)";
	case ORION_ERROR_SEND_DATA_13:                  return "Send data failed (13)";
	case ORION_ERROR_SEND_DATA_14:                  return "Send data failed (14)";
	case ORION_ERROR_SEND_DATA_15:                  return "Send data failed (15)";
	case ORION_ERROR_RECEIVE_DATA_1:                return "Receive data failed (1)";
	case ORION_ERROR_RECEIVE_DATA_2:                return "Receive data failed (2)";
	case ORION_ERROR_RECEIVE_DATA_3:                return "Receive data failed (3)";
	case ORION_ERROR_RECEIVE_DATA_4:                return "Receive data failed (4)";
	case ORION_ERROR_RECEIVE_DATA_5:                return "Receive data failed (5)";
	case ORION_ERROR_RECEIVE_DATA_6:                return "Receive data failed (6)";
	case ORION_ERROR_RECEIVE_DATA_7:                return "Receive data failed (7)";
	case ORION_ERROR_RECEIVE_DATA_8:                return "Receive data failed (8)";
	case ORION_ERROR_RECEIVE_DATA_9:                return "Receive data failed (9)";
	case ORION_ERROR_RECEIVE_DATA_10:               return "Receive data failed (10)";
	case ORION_ERROR_RECEIVE_DATA_11:               return "Receive data failed (11)";
	case ORION_ERROR_RECEIVE_DATA_12:               return "Receive data failed (12)";
	case ORION_ERROR_RECEIVE_DATA_13:               return "Receive data failed (13)";
	case ORION_ERROR_RECEIVE_DATA_14:               return "Receive data failed (14)";
	case ORION_ERROR_RECEIVE_DATA_15:               return "Receive data failed (15)";
	case ORION_ERROR_RECEIVE_DATA_16:               return "Receive data failed (16)";
	case ORION_ERROR_RECEIVE_DATA_17:               return "Receive data failed (17)";
	case ORION_ERROR_RECEIVE_DATA_18:               return "Receive data failed (18)";
	case ORION_ERROR_RECEIVE_DATA_19:               return "Receive data failed (19)";
	case ORION_ERROR_RECEIVE_DATA_20:               return "Receive data failed (20)";
	case ORION_ERROR_RECEIVE_DATA_21:               return "Receive data failed (21)";
	case ORION_ERROR_RECEIVE_DATA_22:               return "Receive data failed (22)";
	case ORION_ERROR_RECEIVE_DATA_23:               return "Receive data failed (23)";
	case ORION_ERROR_RECEIVE_DATA_24:               return "Receive data failed (24)";
	case ORION_ERROR_RECEIVE_DATA_25:               return "Receive data failed (25)";
	case ORION_ERROR_RECEIVE_DATA_26:               return "Receive data failed (26)";
	case ORION_ERROR_RECEIVE_DATA_27:               return "Receive data failed (27)";
	case ORION_ERROR_RECEIVE_DATA_28:               return "Receive data failed (28)";
	case ORION_ERROR_EXPIRED_KEY:                   return "License key has expired";
	case ORION_ERROR_FROZEN_KEY:                    return "License key is frozen";
	case ORION_ERROR_BANNED_KEY:                    return "License key is banned";
	case ORION_ERROR_BRANDING_FORMAT_STRING:        return "Branding format string error";
	case ORION_ERROR_OPEN_REGISTRY_KEY_1:           return "RegOpenKeyEx failed (1)";
	case ORION_ERROR_OPEN_REGISTRY_KEY_2:           return "RegOpenKeyEx failed (2)";
	case ORION_ERROR_OPEN_REGISTRY_KEY_3:           return "RegOpenKeyEx failed (3)";
	case ORION_ERROR_OPEN_REGISTRY_KEY_4:           return "RegOpenKeyEx failed (4)";
	case ORION_ERROR_CLOSE_REGISTRY_KEY_1:          return "RegCloseKey failed (1)";
	case ORION_ERROR_CLOSE_REGISTRY_KEY_2:          return "RegCloseKey failed (2)";
	case ORION_ERROR_CLOSE_REGISTRY_KEY_3:          return "RegCloseKey failed (3)";
	case ORION_ERROR_CLOSE_REGISTRY_KEY_4:          return "RegCloseKey failed (4)";
	case ORION_ERROR_QUERY_REGISTRY_VALUE_1:        return "RegQueryValueEx failed (1)";
	case ORION_ERROR_QUERY_REGISTRY_VALUE_2:        return "RegQueryValueEx failed (2)";
	case ORION_ERROR_QUERY_REGISTRY_VALUE_3:        return "RegQueryValueEx failed (3)";
	case ORION_ERROR_QUERY_REGISTRY_VALUE_4:        return "RegQueryValueEx failed (4)";
	case ORION_ERROR_QUERY_REGISTRY_VALUE_5:        return "RegQueryValueEx failed (5)";
	case ORION_ERROR_QUERY_REGISTRY_VALUE_6:        return "RegQueryValueEx failed (6)";
	case ORION_ERROR_QUERY_REGISTRY_VALUE_7:        return "RegQueryValueEx failed (7)";
	case ORION_ERROR_QUERY_REGISTRY_VALUE_8:        return "RegQueryValueEx failed (8)";
	case ORION_ERROR_QUERY_REGISTRY_VALUE_9:        return "RegQueryValueEx failed (9)";
	case ORION_ERROR_QUERY_REGISTRY_VALUE_10:       return "RegQueryValueEx failed (10)";
	case ORION_ERROR_QUERY_REGISTRY_VALUE_11:       return "RegQueryValueEx failed (11)";
	case ORION_ERROR_QUERY_REGISTRY_VALUE_12:       return "RegQueryValueEx failed (12)";
	case ORION_ERROR_EXECUTE_QUERY:                 return "WMI ExecQuery failed";
	case ORION_ERROR_CLEAR_VARIANT_1:               return "VariantClear failed (1)";
	case ORION_ERROR_CLEAR_VARIANT_2:               return "VariantClear failed (2)";
	case ORION_ERROR_CLEAR_VARIANT_3:               return "VariantClear failed (3)";
	case ORION_ERROR_INITIALIZE_PARSER_1:           return "Parser initialization failed (1)";
	case ORION_ERROR_INITIALIZE_PARSER_2:           return "Parser initialization failed (2)";
	case ORION_ERROR_INITIALIZE_PARSER_3:           return "Parser initialization failed (3)";
	case ORION_ERROR_DEVICE_IO_CONTROL_FILE:        return "DeviceIoControlFile failed";
	case ORION_ERROR_GET_DEVICE_COUNT:              return "Failed to get device count";
	case ORION_ERROR_GET_DEVICE_HANDLE_BY_INDEX:    return "Failed to get device handle by index";
	case ORION_ERROR_GET_DEVICE_UUID:               return "Failed to get device UUID";
	case ORION_ERROR_SHUTDOWN:                      return "Shutdown error";
	case ORION_ERROR_GET_ADAPTERS_INFORMATION_1:    return "GetAdaptersInfo failed (1)";
	case ORION_ERROR_GET_ADAPTERS_INFORMATION_2:    return "GetAdaptersInfo failed (2)";
	case ORION_ERROR_CHANGED_SYSTEM:                return "System configuration has changed";
	case ORION_ERROR_BANNED_SYSTEM:                 return "This system is banned";
	case ORION_ERROR_INVALID_SYSTEM:                return "Invalid system configuration";
	case ORION_ERROR_ENABLED_SECURE_BOOT:           return "Secure Boot must be disabled";
	case ORION_ERROR_DISABLED_VIRTUALIZATION:       return "Virtualization must be enabled";
	case ORION_ERROR_DISABLED_HYPER_V:              return "Hyper-V must be enabled";
	case ORION_ERROR_OPEN_FILE_1:                   return "Failed to open file (1)";
	case ORION_ERROR_OPEN_FILE_2:                   return "Failed to open file (2)";
	case ORION_ERROR_SET_INFORMATION_FILE_1:        return "NtSetInformationFile failed (1)";
	case ORION_ERROR_SET_INFORMATION_FILE_2:        return "NtSetInformationFile failed (2)";
	case ORION_ERROR_CREATE_FILE:                   return "CreateFile failed";
	case ORION_ERROR_WRITE_FILE:                    return "WriteFile failed";
	case ORION_ERROR_REBOOT_SYSTEM:                 return "System reboot failed";
	case ORION_ERROR_CREATE_PROCESS_1:              return "CreateProcess failed (1)";
	case ORION_ERROR_CREATE_PROCESS_2:              return "CreateProcess failed (2)";
	case ORION_ERROR_CREATE_PROCESS_3:              return "CreateProcess failed (3)";
	case ORION_ERROR_DELAY_EXECUTION:               return "NtDelayExecution failed";
	case ORION_ERROR_INVALID_ALLOCATE_ADDRESS_1:    return "Invalid allocation address (1)";
	case ORION_ERROR_INVALID_ALLOCATE_ADDRESS_2:    return "Invalid allocation address (2)";
	case ORION_ERROR_INVALID_LIBRARY_1:             return "Invalid library (1)";
	case ORION_ERROR_INVALID_LIBRARY_2:             return "Invalid library (2)";
	case ORION_ERROR_INVALID_LIBRARY_3:             return "Invalid library (3)";
	case ORION_ERROR_INVALID_LIBRARY_4:             return "Invalid library (4)";
	case ORION_ERROR_INVALID_LIBRARY_5:             return "Invalid library (5)";
	case ORION_ERROR_INVALID_LIBRARY_6:             return "Invalid library (6)";
	case ORION_ERROR_INVALID_LIBRARY_7:             return "Invalid library (7)";
	case ORION_ERROR_INVALID_LIBRARY_8:             return "Invalid library (8)";
	case ORION_ERROR_INVALID_LIBRARY_9:             return "Invalid library (9)";
	case ORION_ERROR_INVALID_LIBRARY_10:            return "Invalid library (10)";
	case ORION_ERROR_INVALID_LIBRARY_11:            return "Invalid library (11)";
	case ORION_ERROR_INVALID_LIBRARY_12:            return "Invalid library (12)";
	case ORION_ERROR_READ_MEMORY_1:                 return "ReadProcessMemory failed (1)";
	case ORION_ERROR_READ_MEMORY_2:                 return "ReadProcessMemory failed (2)";
	case ORION_ERROR_READ_MEMORY_3:                 return "ReadProcessMemory failed (3)";
	case ORION_ERROR_READ_MEMORY_4:                 return "ReadProcessMemory failed (4)";
	case ORION_ERROR_FIND_LIBRARY:                  return "Failed to find library in process";
	case ORION_ERROR_GET_CONTEXT_THREAD:            return "GetThreadContext failed";
	case ORION_ERROR_SET_CONTEXT_THREAD:            return "SetThreadContext failed";
	case ORION_ERROR_RESUME_THREAD:                 return "ResumeThread failed";
	case ORION_ERROR_FLUSH_DNS_RESOLVER_CACHE:      return "DnsFlushResolverCache failed";
	case ORION_ERROR_INITIALIZE_DISM:               return "DismInitialize failed";
	case ORION_ERROR_OPEN_DISM_SESSION:             return "DismOpenSession failed";
	case ORION_ERROR_GET_DISM_FEATURE_INFORMATION:  return "DismGetFeatureInfo failed";
	case ORION_ERROR_DELETE_DISM:                   return "DismDelete failed";
	case ORION_ERROR_CLOSE_DISM_SESSION:            return "DismCloseSession failed";
	case ORION_ERROR_SHUTDOWN_DISM:                 return "DismShutdown failed";
	case ORION_ERROR_GET_THREAD_GROUP_AFFINITY:     return "GetThreadGroupAffinity failed";
	case ORION_ERROR_SET_THREAD_GROUP_AFFINITY_1:   return "SetThreadGroupAffinity failed (1)";
	case ORION_ERROR_SET_THREAD_GROUP_AFFINITY_2:   return "SetThreadGroupAffinity failed (2)";
	case ORION_ERROR_GET_ACTIVE_PROCESSOR_GROUP_COUNT: return "GetActiveProcessorGroupCount failed";
	case ORION_ERROR_GET_ACTIVE_PROCESSOR_COUNT:    return "GetActiveProcessorCount failed";
	case ORION_ERROR_INITIALIZE_HYPERVISOR:         return "Hypervisor initialization failed";
	case ORION_ERROR_NOT_FLASH_DRIVE:               return "Target drive is not a flash drive";
	case ORION_ERROR_INVALID_FIRMWARE_TYPE:         return "Invalid firmware type";
	default:                                        return "Unknown error";
	}
}

struct OrionData
{
	bool is_token;
	char* token_or_key;
	int product_id;
	int progress;
	int error;
};

extern "C" bool orion_load(OrionData* orion_data);
extern "C" int orion_get_download_progress();
extern "C" int orion_get_load_progress();
extern "C" OrionError orion_get_error();
