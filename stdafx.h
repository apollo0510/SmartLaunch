#ifndef LAUNCHER_STD_AFX_HPP_DEFINED
#define LAUNCHER_STD_AFX_HPP_DEFINED

#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <string>
#include <vector>
#include <psapi.h>

#include "syntax_tree.h"

enum Action
{
	Action_Debug,
	Action_Open,
	Action_Close,
	Action_Focus,
	Action_Minimize,
	Action_Restore,
	Action_Maximize,
	Action_Last
};

enum ArgumentIndex
{
	AI_Actions,
	AI_WindowName,
	AI_Executable,
	AI_Parameters
};

typedef std::vector<Action> ActionList;
typedef SyntaxTree<wchar_t,Action> Tree;


bool ParseActions(ActionList& al, int argc, wchar_t * argv[]);
bool SearchWindow();
void ExecuteActions(ActionList& al);

bool IsActiveWindow();
bool ActionFocus();



#endif