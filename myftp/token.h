// file:		token.h for Week 7
// purpose;		to separate a line of characters into a sequence of tokens.
// assumptions:		the tokens in the line are all separated by at least one token separator.
//			the token separators are: space character, tab character and newline character
// author:		HX
// date:		2006.09.14
// last modified:	2006.10.05
// note:		not thoroughly tested therefore it may contain errors

#define MAX_NUM_TOKENS  1000
#define tokenSeparators " \t\n"    // characters that separate tokens

// purpose:
//		to tokenise the command line which is stored in char array "line"
//		as an array of characters.  It breaks up a sequence of characters into
//		a sequence of tokens. It assign the start addresses in array "line" of
//		of the ith token to "token[i]" and it replaces the token separator that
//		immediately follows the ith token with null ('\0') character.
//
// return:
//		1) the number of tokens found in the command line "line", or
//		2) -1,  if array "token" is too small 
//
// assume:	
//		the array passed to the function, token, must have at least MAX_NUM_TOKENS 
//		number of elements
// 
// note:
//		If return value ntokens >= 0, then token[ntokens] is set to NULL. 
// 
int tokenise (char line[], char *token[]);


