#pragma once

// -----------------------------------------------------------------------------
//  ANSI color codes (requires ENABLE_VIRTUAL_TERMINAL_PROCESSING on Windows)
// -----------------------------------------------------------------------------
#define CLR_BORDER   "\033[92m"   // bright green  - borders & headers
#define CLR_OPTION   "\033[93m"   // bright yellow - menu option numbers
#define CLR_RESULT   "\033[96m"   // bright cyan   - algorithm results / output
#define CLR_WARN     "\033[91m"   // bright red    - warnings / errors
#define CLR_BASE     "\033[97m"   // bright white  - base names in network map
#define CLR_EDGE     "\033[92m"   // bright green  - edge arrows --->
#define CLR_RESET    "\033[0m"
