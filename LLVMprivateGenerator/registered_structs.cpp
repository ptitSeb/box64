#include <string>

#define ALL START() \
	/* libc */ \
	STRUCT("_IO_FILE", "a FILE") \
	STRUCT("_G_fpos_t", "a file position") \
	STRUCT("sockaddr", "a socket address") \
	STRUCT("itimerspec", "an itimerspec") \
	STRUCT("timespec", "a timespec") \
	STRUCT("itimerval", "an itimerval") \
	STRUCT("timeval", "a timeval") \
	STRUCT("timex", "a timex") \
	STRUCT("timezone", "a timezone") \
	STRUCT("dirent", "a dirent") \
	STRUCT("dirent64", "a dirent64") \
	STRUCT("__dirstream", "a dir stream") \
	STRUCT("tm", "a time structure (tm)") \
	STRUCT("cmsghdr", "a cmsghdr") \
	STRUCT("msghdr", "a msghdr") \
	STRUCT("rpcent", "an rpcent") \
	STRUCT("random_data", "a random_data structure") \
	STRUCT("drand48_data", "a drand48_data structure") \
	STRUCT("termios", "a termios") \
	STRUCT("iovec", "an iovec") \
	STRUCT("file_handle", "a file handle") \
	STRUCT("lconv", "an lconv") \
	STRUCT("__locale_struct", "a locale structure") \
	STRUCT("aliasent", "an alias") \
	STRUCT("fstab", "an fstab") \
	STRUCT("group", "a group") \
	STRUCT("hostent", "a hostent") \
	STRUCT("protoent", "a protoent") \
	STRUCT("passwd", "a password") \
	STRUCT("spwd", "an spwd") \
	STRUCT("ttyent", "a ttyent") \
	STRUCT("utmp", "an utmp structure") \
	STRUCT("utmpx", "an utmpx structure") \
	STRUCT("ifaddrs", "an ifaddrs structure") \
	STRUCT("statfs", "a statfs structure") \
	STRUCT("statfs64", "a statfs64 structure") \
	STRUCT("statvfs", "a statvfs structure") \
	STRUCT("statvfs64", "a statvfs64 structure") \
	STRUCT("timeb", "a timeb structure") \
	STRUCT("_ftsent", "an _ftsent structure") \
	STRUCT("sysinfo", "a sysinfo structure") \
	STRUCT("rlimit", "an rlimit structure") \
	STRUCT("rlimit64", "an rlimit64 structure") \
	STRUCT("rusage", "an rusage structure") \
	STRUCT("entry", "an entry structure") \
	STRUCT("pollfd", "a pollfd structure") \
	STRUCT("re_pattern_buffer", "a re_pattern_buffer structure") \
	STRUCT("sembuf", "a sembuf structure") \
	STRUCT("tms", "a tms structure") \
	STRUCT("utsname", "an utsname structure") \
	STRUCT("utimbuf", "an utimbuf structure") \
	STRUCT2("__va_list_tag", "__va_list_tag (aka, a va_list)", 'A') \
	/* ncurses */ \
	STRUCT("_win_st", "a _win_st structure") \
	STRUCT("MEVENT", "an MEVENT structure") \
	TYPEDEF("cchar_t", "a cchar_t") \
	/* zlib */ \
	STRUCT("gz_header_s", "a gz_header_s structure") \
	STRUCT("gzFile_s", "a gzFile_s structure") \
	STRUCT("z_stream_s", "a z_stream_s structure") \
	\
	END()

#define START()
#define STRUCT(s, ret) if (str == s) { return 'p'; } else
#define STRUCT2(s, ret, c) if (str == s) { return c; } else
#define TYPEDEF(s, ret) if (str == s) { return 'p'; } else
#define END() { return 0; }
char ptr2char(const std::string &str) {
	/*if ((str == "_IO_FILE")
	 || (str == "_G_fpos_t")
	 || (str == "sockaddr")
	 || (str == "itimerspec")
	 || (str == "timespec")
	 || (str == "itimerval")
	 || (str == "timeval")
	 || (str == "timex")
	 || (str == "timezone")
	 || (str == "dirent")
	 || (str == "dirent64")
	 || (str == "__dirstream")
	 || (str == "tm")
	 || (str == "cmsghdr")
	 || (str == "msghdr")
	 || (str == "rpcent")
	 || (str == "random_data")
	 || (str == "drand48_data")
	 || (str == "termios")
	 || (str == "iovec")
	 || (str == "file_handle")
	 || (str == "lconv")
	 || (str == "__locale_struct")
	 || (str == "aliasent")
	 || (str == "fstab")
	 || (str == "group")
	 || (str == "hostent")
	 || (str == "protoent")
	 || (str == "passwd")
	 || (str == "spwd")
	 || (str == "ttyent")
	 || (str == "utmp")
	 || (str == "utmpx")
	 || (str == "ifaddrs")
	 || (str == "statfs")
	 || (str == "statfs64")
	 || (str == "statvfs")
	 || (str == "timeb")
	 || (str == "_ftsent")
	 || (str == "sysinfo")
	 || (str == "rlimit")
	 || (str == "rlimit64")
	 || (str == "rusage")
	 || (str == "entry")
	 || (str == "pollfd")
	 || (str == "re_pattern_buffer")
	 || (str == "sembuf")
	 || (str == "tms")
	 || (str == "utsname")
	 || (str == "utimbuf")
	// ncurses
	 || (str == "_win_st")
	 
	 || (str == "cchar_t")
	) {
		// FILE*, fpos_t*, ...
		return 'p';
	} else if (str == "__va_list_tag") {
		return 'A';
	} else {
		return 0;
	}*/
	ALL
}
#undef END
#undef TYPEDEF
#undef STRUCT2
#undef STRUCT
#undef START

#define START()
#define STRUCT(s, ret) if (str == s) { return ret; } else
#define STRUCT2(s, ret, c) if (str == s) { return ret; } else
#define TYPEDEF(s, ret) if (str == s) { return ret; } else
#define END() return "";
const char *ptr2str(const std::string &str) {
	/*if (str == "_IO_FILE") {
		return "a FILE";
	} else if (str == "_G_fpos_t") {
		return "a file position";
	} else if (str == "sockaddr") {
		return "a socket address";
	} else if (str == "itimerspec") {
		return "an itimerspec";
	} else if (str == "timespec") {
		return "a timespec";
	} else if (str == "itimerval") {
		return "an itimerval";
	} else if (str == "timeval") {
		return "a timeval";
	} else if (str == "timex") {
		return "a timex";
	} else if (str == "timezone") {
		return "a timezone";
	} else if (str == "dirent") {
		return "a dirent";
	} else if (str == "dirent64") {
		return "a dirent64";
	} else if (str == "__dirstream") {
		return "a dir stream";
	} else if (str == "tm") {
		return "a time structure (tm)";
	} else if (str == "cmsghdr") {
		return "a cmsghdr";
	} else if (str == "msghdr") {
		return "a msghdr";
	} else if (str == "rpcent") {
		return "an rpcent";
	} else if (str == "random_data") {
		return "a random_data structure";
	} else if (str == "drand48_data") {
		return "a drand48_data structure";
	} else if (str == "termios") {
		return "a termios";
	} else if (str == "iovec") {
		return "an iovec";
	} else if (str == "file_handle") {
		return "a file handle";
	} else if (str == "lconv") {
		return "an lconv";
	} else if (str == "__locale_struct") {
		return "a locale structure";
	} else if (str == "aliasent") {
		return "an alias";
	} else if (str == "fstab") {
		return "an fstab";
	} else if (str == "group") {
		return "a group";
	} else if (str == "hostent") {
		return "a hostent";
	} else if (str == "protoent") {
		return "a protoent";
	} else if (str == "passwd") {
		return "a password";
	} else if (str == "spwd") {
		return "an spwd";
	} else if (str == "ttyent") {
		return "a ttyent";
	} else if (str == "utmp") {
		return "an utmp structure";
	} else if (str == "utmpx") {
		return "an utmpx structure";
	} else if (str == "ifaddrs") {
		return "an ifaddrs structure";
	} else if (str == "statfs") {
		return "a statfs structure";
	} else if (str == "statfs64") {
		return "a statfs64 structure";
	} else if (str == "statvfs") {
		return "a statvfs structure";
	} else if (str == "statvfs64") {
		return "a statvfs64 structure";
	} else if (str == "timeb") {
		return "a timeb structure";
	} else if (str == "_ftsent") {
		return "an _ftsent structure";
	} else if (str == "sysinfo") {
		return "a sysinfo structure";
	} else if (str == "rlimit") {
		return "an rlimit structure";
	} else if (str == "rlimit64") {
		return "an rlimit64 structure";
	} else if (str == "rusage") {
		return "an rusage structure";
	} else if (str == "entry") {
		return "an entry structure";
	} else if (str == "pollfd") {
		return "a pollfd structure";
	} else if (str == "re_pattern_buffer") {
		return "a re_pattern_buffer structure";
	} else if (str == "sembuf") {
		return "a sembuf structure";
	} else if (str == "tms") {
		return "a tms structure";
	} else if (str == "utsname") {
		return "an utsname structure";
	} else if (str == "utimbuf") {
		return "an utimbuf structure";
	} else if (str == "__va_list_tag") {
		return "__va_list_tag (aka, a va_list)";
	// ncurses
	} else if (str == "_win_st") {
		return "a _win_st structure";
		
	} else if (str == "cchar_t") {
		return "a cchar_t";
	} else return "";*/
	ALL
}
#undef END
#undef TYPEDEF
#undef STRUCT2
#undef STRUCT
#undef START
