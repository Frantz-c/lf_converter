# lf_converter

the Makefile run 'gcc -O3 lf_converter.c -o lfconv' command
run without arguments print the usage


usage

    ./lfconv [option] files...
    option:
        --windows or -w : convert to windows linefeed format (\r\n)
        --unix or -u.   : convert to unix linefeed format (\n)
