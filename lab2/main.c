#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <grp.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h>

static int err_code;

/*
 * here are some function signatures and macros that may be helpful.
 */

void handle_error(char* fullname, char* action);
bool test_file(char* pathandname);
bool is_dir(char* pathandname);
const char* ftype_to_str(mode_t mode);
void list_file(char* pathandname, char* name, bool list_long);
void list_dir(char* dirname, bool list_long, bool list_all, bool recursive);

/*
 * You can use the NOT_YET_IMPLEMENTED macro to error out when you reach parts
 * of the code you have not yet finished implementing.
 */
#define NOT_YET_IMPLEMENTED(msg)                  \
    do {                                          \
        printf("Not yet implemented: " msg "\n"); \
        exit(255);                                \
    } while (0)

/*
 * PRINT_ERROR: This can be used to print the cause of an error returned by a
 * system call. It can help with debugging and reporting error causes to
 * the user. Example usage:
 *     if ( error_condition ) {
 *        PRINT_ERROR();
 *     }
 */
#define PRINT_ERROR(progname, what_happened, pathandname)               \
    do {                                                                \
        printf("%s: %s %s: %s\n", progname, what_happened, pathandname, \
               strerror(errno));                                        \
    } while (0)

/* PRINT_PERM_CHAR:
 *
 * This will be useful for -l permission printing.  It prints the given
 * 'ch' if the permission exists, or "-" otherwise.
 * Example usage:
 *     PRINT_PERM_CHAR(sb.st_mode, S_IRUSR, "r");
 */
#define PRINT_PERM_CHAR(mode, mask, ch) printf("%s", (mode & mask) ? ch : "-");

/*
 * Get username for uid. Return 1 on failure, 0 otherwise.
 */
static int uname_for_uid(uid_t uid, char* buf, size_t buflen) {
    struct passwd* p = getpwuid(uid);
    if (p == NULL) {
        return 1;
    }
    strncpy(buf, p->pw_name, buflen);
    return 0;
}

/*
 * Get group name for gid. Return 1 on failure, 0 otherwise.
 */
static int group_for_gid(gid_t gid, char* buf, size_t buflen) {
    struct group* g = getgrgid(gid);
    if (g == NULL) {
        return 1;
    }
    strncpy(buf, g->gr_name, buflen);
    return 0;
}

/*
 * Format the supplied `struct timespec` in `ts` (e.g., from `stat.st_mtime`) as a
 * string in `char *out`. Returns the length of the formatted string (see, `man
 * 3 strftime`).
 */
static size_t date_string(struct timespec* ts, char* out, size_t len) {
    struct timespec now;
    timespec_get(&now, TIME_UTC);
    struct tm* t = localtime(&ts->tv_sec);
    if (now.tv_sec < ts->tv_sec) {
        // Future time, treat with care.
        return strftime(out, len, "%b %e %Y", t);
    } else {
        time_t difference = now.tv_sec - ts->tv_sec;
        if (difference < 31556952ull) {
            return strftime(out, len, "%b %e %H:%M", t);
        } else {
            return strftime(out, len, "%b %e %Y", t);
        }
    }
}

/*
 * Print help message and exit.
 */
static void help() {
    /* TODO: add to this */
    printf("ls: List files\n");
    printf("\t--help: Print this help\n");
    exit(0);
}

/*
 * call this when there's been an error.
 * The function should:
 * - print a suitable error message (this is already implemented)
 * - set appropriate bits in err_code
 */
void handle_error(char* what_happened, char* fullname) {
    PRINT_ERROR("ls", what_happened, fullname);

    // TODO: your code here: inspect errno and set err_code accordingly.
	err_code|=0x80;
	switch(errno){
		case ENOENT: err_code|=1; break;
		case EACCES: err_code|=2; break;
		default: err_code|=2; break;
	}
    return;
}

/*
 * test_file():
 * test whether stat() returns successfully and if not, handle error.
 * Use this to test for whether a file or dir exists
 */
bool test_file(char* pathandname) {
    struct stat sb;
    if (stat(pathandname, &sb)) {
        handle_error("cannot access", pathandname);
        return false;
    }
    return true;
}

/*
 * is_dir(): tests whether the argument refers to a directory.
 * precondition: test_file() returns true. that is, call this function
 * only if test_file(pathandname) returned true.
 */
bool is_dir(char* pathandname) {
    /* TODO: fillin */
	struct stat info;
	stat(pathandname, &info);
	return (S_IFMT&info.st_mode)==S_IFDIR;
}

/* convert the mode field in a struct stat to a file type, for -l printing */
const char* ftype_to_str(mode_t mode) {
    /* TODO: fillin */
	static char modestr[11];
	modestr[0]=((mode&S_IFMT)==S_IFDIR)?'d':'-';
	//owner
	modestr[1]=(mode&0400)?'r':'-';
	modestr[2]=(mode&0200)?'w':'-';
	modestr[3]=(mode&0100)?'x':'-';
	//group
	modestr[4]=(mode&040)?'r':'-';
	modestr[5]=(mode&020)?'w':'-';
	modestr[6]=(mode&010)?'x':'-';
	//others
	modestr[7]=(mode&04)?'r':'-';
	modestr[8]=(mode&02)?'w':'-';
	modestr[9]=(mode&01)?'x':'-';

	modestr[10]=0;
	return modestr;
}

/* list_file():
 * implement the logic for listing a single file.
 * This function takes:
 *   - pathandname: the directory name plus the file name.
 *   - name: just the name "component".
 *   - list_long: a flag indicated whether the printout should be in
 *   long mode.
 *
 *   The reason for this signature is convenience: some of the file-outputting
 *   logic requires the full pathandname (specifically, testing for a directory
 *   so you can print a '/' and outputting in long mode), and some of it
 *   requires only the 'name' part. So we pass in both. An alternative
 *   implementation would pass in pathandname and parse out 'name'.
 */
void list_file(char* pathandname, char* name, bool list_long) {
    /* TODO: fill in*/
	if(list_long){
		struct stat info;
		if(stat(pathandname, &info)==-1){
			handle_error("cannot open file", name);
			return;
		}
		//date string
#define DATE_BUF_SIZE 16
		char datestr[DATE_BUF_SIZE];
		date_string(&info.st_mtim, datestr, DATE_BUF_SIZE);
#define ID_BUF_SIZE 10
		char uname[ID_BUF_SIZE], group[ID_BUF_SIZE];
		uname_for_uid(info.st_uid, uname, ID_BUF_SIZE);
		group_for_gid(info.st_gid, group, ID_BUF_SIZE);
		//isDir, modes, mode number, user, group, modified time, size,
		printf("%s %4d %*s %*s %*s %6ld ", ftype_to_str(info.st_mode), (info.st_mode&S_IFMT)>>14, ID_BUF_SIZE, uname, ID_BUF_SIZE, group, DATE_BUF_SIZE, datestr, info.st_size);
#undef ID_BUF_SIZE
	}
	printf("%s\n", name);
}

/* list_dir():
 * implement the logic for listing a directory.
 * This function takes:
 *    - dirname: the name of the directory
 *    - list_long: should the directory be listed in long mode?
 *    - list_all: are we in "-a" mode?
 *    - recursive: are we supposed to list sub-directories?
 */
void list_dir(char* dirname, bool list_long, bool list_all, bool recursive) {
    /* TODO: fill in
     *   You'll probably want to make use of:
     *       opendir()
     *       readdir()
     *       list_file()
     *       snprintf() [to make the 'pathandname' argument to
     *          list_file(). that requires concatenating 'dirname' and
     *          the 'd_name' portion of the dirents]
     *       closedir()
     *   See the lab description for further hints
     */
	DIR* dir=opendir(dirname);
	if(dir==NULL){
		handle_error("cannot open directory",dirname);
		return;
	}
	struct dirent* entry;
	while((entry=readdir(dir))){
		char pathandname[512];
		if(list_all||entry->d_name[0]!='.'){
			snprintf(pathandname, 512, "%s/%s", dirname, entry->d_name);
			list_file(pathandname, entry->d_name, list_long);
		}
	}
	closedir(dir);
}

int main(int argc, char* argv[]) {
    // This needs to be int since C does not specify whether char is signed or
    // unsigned.
    int opt;
    err_code = 0;
    bool list_long = false, list_all = false, list_recursive = false;
    // We make use of getopt_long for argument parsing, and this
    // (single-element) array is used as input to that function. The `struct
    // option` helps us parse arguments of the form `--FOO`. Refer to `man 3
    // getopt_long` for more information.
    struct option opts[] = {
        {.name = "help", .has_arg = 1, .flag = NULL, .val = '\a'}};

    // This loop is used for argument parsing. Refer to `man 3 getopt_long` to
    // better understand what is going on here.
    while ((opt = getopt_long(argc, argv, "1laRr", opts, NULL)) != -1) {
        switch (opt) {
            case '\a':
                // Handle the case that the user passed in `--help`. (In the
                // long argument array above, we used '\a' to indicate this
                // case.)
				printf("--help, arg=%s\n", optarg);
                help();
                break;
            case '1':
                // Safe to ignore since this is default behavior for our version
                // of ls.
                break;
			case 'l':
				list_long=true;
				break;
            case 'a':
                list_all = true;
                break;
                // TODO: you will need to add items here to handle the
                // cases that the user enters "-l" or "-R"
			case 'r':
			case 'R':
				list_recursive=true;
				break;
            default:
                printf("Unimplemented flag %d\n", opt);
                break;
        }
    }

    // TODO: Replace this.
	//if there is no directory specified
    if (optind >= argc) {
		list_dir(".",list_long,list_all,list_recursive);
    }else{
		for (int i = optind; i < argc; i++) {
			printf("%s:\n", argv[i]);
			list_dir(argv[i],list_long,list_all,list_recursive);
			putchar('\n');
		}
	}

    exit(err_code);
}
