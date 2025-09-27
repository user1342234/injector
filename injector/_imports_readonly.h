#ifndef _IMPORTS_READONLY_H
#define _IMPORTS_READONLY_H
typedef PVOID IMPORT_VAR;
#define _IMPORT(name) pfn##name
#define _IMPORT_VAR(name) pvar##name
// IMPORTS
#define INITIALIZE_IMPORT(import_name, pattern, mask) IMPORT imp##import_name{pattern, mask};
#define RESOLVE_IMPORT(import_name, file_base) import_name = (_IMPORT(import_name))FindPattern(file_base, imp##import_name.pattern, imp##import_name.mask);
#define DEFINE_IMPORT(pattern, mask, ret_type, name, ...) \
	typedef ret_type (*_IMPORT(name))(__VA_ARGS__); \
	_IMPORT(name) name = nullptr; \
	INITIALIZE_IMPORT(name, pattern, mask)

#define DECLARE_IMPORT(ret_type, name, ...) extern ret_type (*(name))(__VA_ARGS__);

// IMPORTing variables
#define DEFINE_VAR(pattern, mask, name) \
	IMPORT_VAR _IMPORT_VAR(name); \
	INITIALIZE_IMPORT(name, pattern, mask);

#define DECLARE_VAR(name) extern PVOID _IMPORT_VAR(name);
#define RESOLVE_VAR(name, instr_size, file_base) do { \
	UINT64 address = FindPattern(file_base, imp##name.pattern, imp##name.mask); \
	_IMPORT_VAR(name) = RESOLVE_POINTER(address, instr_size); } while (0);

#endif