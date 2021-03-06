.func ispunct iswpunct
.synop begin
.funcw iswpunct
#include <ctype.h>
int ispunct( int c );
.ixfunc2 '&CharTest' &func
.if &'length(&wfunc.) ne 0 .do begin
#include <wctype.h>
int iswpunct( wint_t c );
.ixfunc2 '&CharTest' &wfunc
.ixfunc2 '&Wide' &wfunc
.do end
.synop end
.desc begin
The
.id &func.
function tests for any punctuation character such as a comma
(,) or a period (.).
.if &'length(&wfunc.) ne 0 .do begin
.np
The
.id &wfunc.
function is similar to
.id &func.
except that it accepts a
wide-character argument.
.do end
.desc end
.return begin
The
.id &func.
function returns a non-zero value when the argument is a
punctuation character.
.if &'length(&wfunc.) ne 0 .do begin
The
.id &wfunc.
function returns a non-zero value when the argument is a
printable wide character that is neither the space wide character nor
a wide character for which
.kw iswalnum
is true.
.do end
Otherwise, zero is returned.
.return end
.see begin
.im seeis &function.
.see end
.exmp begin
#include <stdio.h>
#include <ctype.h>

char chars[] = {
    'A',
    '!',
    '.',
    ',',
    ':',
    ';'
};
.exmp break
#define SIZE sizeof( chars ) / sizeof( char )
.exmp break
void main()
{
    int   i;
.exmp break
    for( i = 0; i < SIZE; i++ ) {
        printf( "Char %c is %sa punctuation character\n",
                chars[i],
                ( ispunct( chars[i] ) ) ? "" : "not " );
    }
}
.exmp output
Char A is not a punctuation character
Char ! is a punctuation character
Char . is a punctuation character
Char , is a punctuation character
Char : is a punctuation character
Char ; is a punctuation character
.exmp end
.class ANSI
.system
