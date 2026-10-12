%option noyywrap
%{
/*****************声明和选项设置  begin*****************/
#include <stdio.h>
#include <stdlib.h>

#include "syntax_tree.h"
#include "syntax_analyzer.h"

int lines=1;
int pos_start=1;
int pos_end=1;

void pass_node(char *text){
     yylval.node = new_syntax_tree_node(text);
}

/*****************声明和选项设置  end*****************/

%}

%x COMMENT

%%
 /* to do for students */
 /* two cases for you, pass_node will send flex's token to bison */
\+ 	{pos_start = pos_end; pos_end += 1; pass_node(yytext); return ADD;}
\-	{pos_start = pos_end; pos_end += 1; pass_node(yytext); return SUB;}
\*	{pos_start = pos_end; pos_end += 1; pass_node(yytext); return MUL;}
\/	{pos_start = pos_end; pos_end += 1; pass_node(yytext); return DIV;}
\<	{pos_start = pos_end; pos_end += 1; pass_node(yytext); return LT;}
\<=	{pos_start = pos_end; pos_end += 2; pass_node(yytext); return LTE;}
\>	{pos_start = pos_end; pos_end += 1; pass_node(yytext); return GT;}
\>=	{pos_start = pos_end; pos_end += 2; pass_node(yytext); return GTE;}
==	{pos_start = pos_end; pos_end += 2; pass_node(yytext); return EQ;}
!=	{pos_start = pos_end; pos_end += 2; pass_node(yytext); return NEQ;}
=	{pos_start = pos_end; pos_end += 1; pass_node(yytext); return ASSIGN;}
;	{pos_start = pos_end; pos_end += 1; pass_node(yytext); return SEMICOLON;}
,	{pos_start = pos_end; pos_end += 1; pass_node(yytext); return COMMA;}
 /* 括号与大括号 */
 \(	{pos_start = pos_end; pos_end += 1; pass_node(yytext); return LPARENTHESE;}
 \)	{pos_start = pos_end; pos_end += 1; pass_node(yytext); return RPARENTHESE;}
 \[	{pos_start = pos_end; pos_end += 1; pass_node(yytext); return LBRACKET;}
 \]	{pos_start = pos_end; pos_end += 1; pass_node(yytext); return RBRACKET;}
 \{	{pos_start = pos_end; pos_end += 1; pass_node(yytext); return LBRACE;}
 \}	{pos_start = pos_end; pos_end += 1; pass_node(yytext); return RBRACE;}

 /* 关键字 */
 else	{pos_start = pos_end; pos_end += 4; pass_node(yytext); return ELSE;}
 if	{pos_start = pos_end; pos_end += 2; pass_node(yytext); return IF;}
 int	{pos_start = pos_end; pos_end += 3; pass_node(yytext); return INT;}
 return	{pos_start = pos_end; pos_end += 6; pass_node(yytext); return RETURN;}
 void	{pos_start = pos_end; pos_end += 4; pass_node(yytext); return VOID;}
 while	{pos_start = pos_end; pos_end += 5; pass_node(yytext); return WHILE;}
 float	{pos_start = pos_end; pos_end += 5; pass_node(yytext); return FLOAT;}

 /* 标识符只由字母组成，数字、下划线都不是合法字符 */
 [a-zA-Z]+	{pos_start = pos_end; pos_end += yyleng; pass_node(yytext); return IDENTIFIER;}

 /* 数值：先写 INTEGER，浮点数的模式更长，flex 按最长匹配优先会选择 FLOATPOINT */
 [0-9]+	{pos_start = pos_end; pos_end += yyleng; pass_node(yytext); return INTEGER;}
 [0-9]+\.[0-9]*	{pos_start = pos_end; pos_end += yyleng; pass_node(yytext); return FLOATPOINT;}
 [0-9]*\.[0-9]+	{pos_start = pos_end; pos_end += yyleng; pass_node(yytext); return FLOATPOINT;}

 /* 空白与换行只维护列号与行号，不返回 token */
 [ \t\r]+	{pos_start = pos_end; pos_end += yyleng;}
 \n	{lines++; pos_start = pos_end = 1;}

 /* 注释用起始状态切换到 COMMENT，注释内的换行同样要计数 */
 "/*"	{pos_start = pos_end; pos_end += 2; BEGIN(COMMENT);}
 <COMMENT>"*/"	{pos_end += 2; BEGIN(INITIAL);}
 <COMMENT>\n	{lines++; pos_start = pos_end = 1;}
 <COMMENT>.	{pos_end++;}


. { pos_start = pos_end; pos_end++; return ERROR; }

 /****请在此补全所有flex的模式与动作  end******/
%%
