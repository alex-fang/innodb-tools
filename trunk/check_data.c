#include <check_data.h>
#include <print_data.h>

// Linux has no isnumber function? (Debian 4.0 has no such function)
#ifndef isnumber
inline int isnumber(char c) {
    return (isdigit(c) || c == '.' || c == '-');
}
#endif

/*******************************************************************/
inline ulonglong make_ulonglong(dulint x) {
	ulonglong lx = x.high;
	lx <<= 32;
	lx += x.low;
	return lx;
}

/*******************************************************************/
inline longlong make_longlong(dulint x) {
	longlong lx = x.high;
	lx <<= 32;
	lx += x.low;
	return lx;
}

/*******************************************************************/
inline ibool check_datetime(ulonglong ldate) {
	int year, month, day, hour, min, sec;
	
	ldate &= ~(1ULL << 63);

	if (ldate == 0) return TRUE;
	if (debug) printf("DATE=OK ");

	sec = ldate % 100; ldate /= 100;
    if (debug) printf("SEC(%d)", sec);
	if (sec < 0 || sec > 59) return FALSE;
	if (debug) printf("=OK ");
	
	min = ldate % 100; ldate /= 100;
    if (debug) printf("MIN(%d)", min);
	if (min < 0 || min > 59) return FALSE;
	if (debug) printf("=OK ");
	
	hour = ldate % 100; ldate /= 100;
    if (debug) printf("HOUR(%d)", hour);
	if (hour < 0 || hour > 23) return FALSE;
	if (debug) printf("=OK ");

	day = ldate % 100; ldate /= 100;
    if (debug) printf("DAY(%d)", day);
	if (day < 0 || day > 31) return FALSE;
	if (debug) printf("=OK ");

	month = ldate % 100; ldate /= 100;
    if (debug) printf("MONTH(%d)", month);
	if (month < 0 || month > 12) return FALSE;
	if (debug) printf("=OK ");

	year = ldate % 10000;
    if (debug) printf("YEAR(%d)", year);
	if (year < 1950 || year > 2050) return FALSE;
	if (debug) printf("=OK ");

	return TRUE;
}

/*******************************************************************/
inline ibool check_char_ascii(char *value, ulint len) {
	char *p = value;
	if (!len) return TRUE;
	do { 
		if (!isprint(*p) && *p != '\n' && *p != '\t' && *p != '\r') return FALSE;
	} while (++p < value + len);
	return TRUE;
}

/*******************************************************************/
inline ibool check_char_digits(char *value, ulint len) {
	char *p = value;
	if (!len) return TRUE;
	do { 
		if (!isnumber(*p)) return FALSE;
	} while (++p < value + len);
	return TRUE;
}

/*******************************************************************/
ibool check_field_limits(field_def_t *field, byte *value, ulint len) {
	long long int int_value;
	unsigned long long int uint_value;
	
	switch (field->type) {
		case FT_INT:
			int_value = get_int_value(field, value);
			if (debug) printf("INT(%i)=%lli ", field->fixed_length, int_value);
			if (int_value < field->limits.int_min_val) {
    			if (debug) printf(" is less than %lli ", field->limits.int_min_val);
			    return FALSE;
			}
			if (int_value > field->limits.int_max_val) {
    			if (debug) printf(" is more than %lli ", field->limits.int_max_val);
			    return FALSE;
			}
			break;

		case FT_UINT:
			uint_value = get_uint_value(field, value);
			if (debug) printf("UINT(%i)=%llu ", field->fixed_length, uint_value);
			if (uint_value < field->limits.uint_min_val) {
    			if (debug) printf(" is less than %llu ", field->limits.uint_min_val);
			    return FALSE;
			}
			if (uint_value > field->limits.uint_max_val) {
    			if (debug) printf(" is more than %llu ", field->limits.uint_max_val);
			    return FALSE;
			}
			break;

		case FT_TEXT:
		case FT_CHAR:
			if (debug) {
				if (len != UNIV_SQL_NULL) {
					if (len <= 30) {
						ut_print_buf(stdout, value, len);
					} else {
						ut_print_buf(stdout, value, 30);
						printf("...(truncated)");
					}
				} else {
					printf("SQL NULL ");
				}				
			}
			if (len < field->limits.char_min_len) return FALSE;
			if (len > field->limits.char_max_len) return FALSE;
			if (field->limits.char_ascii_only && !check_char_ascii((char*)value, len)) return FALSE;
			if (field->limits.char_digits_only && !check_char_digits((char*)value, len)) return FALSE;
			break;

		case FT_DATE:
		case FT_DATETIME:
			if (!check_datetime(make_longlong(mach_read_from_8(value)))) return FALSE;
			break;
		
		case FT_ENUM:
			int_value = get_int_value(field, value);
			if (debug) printf("ENUM=%lli ", int_value);
			if (int_value < 1 || int_value > field->limits.enum_values_count) return FALSE;
			break;
	}

	return TRUE;
}