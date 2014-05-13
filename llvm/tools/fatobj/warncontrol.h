// It's worth to have all potentially useful warnings enabled.
// However, some external headers cause some warnings, so we have to disable them,
// because warnings are treated as errors in our build system.
//
// Each odd inclusion of this header disables header-induced warnings.
// Each even inclusion enables warnings.

// flip-flop behavior: disable-enable-...
#ifndef WARNCONTROL_DISABLED_STATE
  #define WARNCONTROL_DISABLED_STATE
#else
  #undef WARNCONTROL_DISABLED_STATE
#endif

// implementation for MS compiler
#ifdef _MSC_VER

#ifdef WARNCONTROL_DISABLED_STATE
#pragma message (__FILE__ ": Some warnings disabled")
  // These are disabled only during compilation of headers
  #pragma warning(disable:4986) //exception specification does not match previous declaration
  #pragma warning(disable:4061) //enumerator 'x' in switch of enum 'y' is not explicitly handled by a case label
  #pragma warning(disable:4512) //assignment operator could not be generated
  #pragma warning(disable:4625) //copy constructor could not be generated because a base class copy constructor is in accessible
  #pragma warning(disable:4626) //assignment operator could not be generated because a base class assignment operator is inaccessible
  #pragma warning(disable:4100) //unreferenced formal parameter
  #pragma warning(disable:4350) //behavior change: 'function template' is called instead of 'function'
  #pragma warning(disable:4711) //function 'f' selected for automatic inline expansion
  // These are disabled forever:
  #pragma warning(disable:4514) //'function' : unreferenced inline function has been removed
  #pragma warning(disable:4710) //'function' : function not inlined
  #pragma warning(disable:4820) //'bytes' bytes padding added after construct 'member_name'
  #pragma warning(disable:4365) //'action' : conversion from 'type_1' to 'type_2', signed/unsigned mismatch
  #pragma warning(disable:4668) //'symbol' is not defined as a preprocessor macro, replacing with '0' for 'directives'
#else
#pragma message (__FILE__ ": Warnings re-enabled")
  // Re-enable warnings after headers processed
  #pragma warning(error:4986) //exception specification does not match previous declaration
  #pragma warning(error:4061) //enumerator 'x' in switch of enum 'y' is not explicitly handled by a case label
  #pragma warning(error:4512) //assignment operator could not be generated
  #pragma warning(error:4625) //copy constructor could not be generated because a base class copy constructor is in accessible
  #pragma warning(error:4626) //assignment operator could not be generated because a base class assignment operator is inaccessible
  #pragma warning(error:4100) //unreferenced formal parameter
  #pragma warning(error:4350) //behavior change: 'function template' is called instead of 'function'
  // These warnings are disabled by default by MSVC, but useful:
  #pragma warning(error:4296) // expr is always false
  #pragma warning(error:4189) // 'identifier' : local variable is initialized but not referenced
#endif

#endif //_MSC_VER
