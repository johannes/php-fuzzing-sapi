/*
  +----------------------------------------------------------------------+
  | Copyright (c) 2017, Johannes Schlüter <johannes@schlueters.de>       |
  | All rights reserved.                                                 |
  +----------------------------------------------------------------------+
  | Redistribution and use in source and binary forms, with or without   |
  | modification, are permitted provided that the conditions which are   |
  | bundled with this package in the file LICENSE.                       |
  | This product includes PHP software, freely available from            |
  | <http://www.php.net/software/>                                       |
  +----------------------------------------------------------------------+
*/

#include <main/php.h>
#include <main/php_main.h>
#include <main/SAPI.h>
#include <ext/standard/info.h>
#include <ext/standard/php_var.h>
#include <main/php_variables.h>
#ifdef JO0
#include <ext/standard/php_smart_str.h>
#endif

#include "fuzzer.h"

#include "fuzzer-sapi.h"

int fuzzer_do_parse(zend_file_handle *file_handle, char *filename)
{
	int retval = FAILURE; /* failure by default */

	SG(options) |= SAPI_OPTION_NO_CHDIR;
	SG(request_info).argc=0;
	SG(request_info).argv=NULL;

	if (php_request_startup(TSRMLS_C)==FAILURE) {
		php_module_shutdown(TSRMLS_C);
		return FAILURE;
	}

	SG(headers_sent) = 1;
	SG(request_info).no_headers = 1;
	php_register_variable("PHP_SELF", filename, NULL TSRMLS_CC);

	zend_first_try {
		zend_compile_file(file_handle, ZEND_REQUIRE);
		//retval = php_execute_script(file_handle TSRMLS_CC);
	} zend_end_try();

	php_request_shutdown((void *) 0);

	return (retval == SUCCESS) ? SUCCESS : FAILURE;
}

int fuzzer_do_request(zend_file_handle *file_handle, char *filename);
int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
	char *s = malloc(Size+1);
	memcpy(s, Data, Size);
	s[Size] = '\0';

	zend_file_handle file_handle;
	file_handle.filename = "fuzzer.php";
	file_handle.opened_path = NULL;
	file_handle.free_filename = 0;
	file_handle.handle.stream.handle = NULL;
	file_handle.handle.stream.reader = (zend_stream_reader_t)_php_stream_read;
	file_handle.handle.stream.fsizer = NULL;
	file_handle.handle.stream.isatty = 0;
	file_handle.handle.stream.closer   = NULL;
	file_handle.handle.stream.mmap.buf = s;
	file_handle.handle.stream.mmap.len = Size;

	//fuzzer_do_parse(&file_handle, "fuzzer.php");
	fuzzer_do_request(&file_handle, "fuzzer.php");

	free(s);
	return 0;
}

int LLVMFuzzerInitialize(int *argc, char ***argv) {
	fuzzer_init_php();

	/* fuzzer_shutdown_php(); */
	return 0;
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
