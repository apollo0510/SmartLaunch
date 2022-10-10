#include "stdafx.h"

/***********************************************************************************************
* we are lazy. this is sparta !
************************************************************************************************/

int            g_argc         = 0;
wchar_t**      g_argv         = nullptr;

const wchar_t* g_WindowName   = nullptr; // just for easier debugging
HWND           g_window       = 0;
LONG           g_window_style = 0;
HWND           g_ActiveWindow = GetActiveWindow();
bool           g_Debug        = false;

const wchar_t* pLauncher = L"Launcher";

/***********************************************************************************************
* the token tree for our parser
************************************************************************************************/

Tree::Init g_SyntaxInit[]=
{
    { L"DEBUG",Action_Debug },
    { L"OPEN" ,Action_Open  },
    { L"CLOSE",Action_Close },
    { L"FOCUS",Action_Focus },
    { L"MINIMIZE",Action_Minimize },
    { L"RESTORE",Action_Restore },
    { L"MAXIMIZE",Action_Maximize },
    { nullptr,Action_Last },
};

Tree  g_SyntaxTree(g_SyntaxInit);

/***********************************************************************************************
* main program entry point
************************************************************************************************/

int WINAPI wWinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,PWSTR lpCmdline,int nShowCmd)
{
    g_argv = CommandLineToArgvW(lpCmdline, &g_argc);
    ActionList actions;
    if (ParseActions(actions, g_argc, g_argv))
    {
        if (g_argc > AI_WindowName)
        {
            g_WindowName = g_argv[AI_WindowName];
            SearchWindow();
            ExecuteActions(actions);
        }
        else
            MessageBox(0, L"Missing window name", pLauncher, MB_OK);
    }
    return 0;
}

/***********************************************************************************************
* parse the given action string
************************************************************************************************/

bool ParseActions(ActionList& al,int argc, wchar_t* argv[])
{
    if(argc>= AI_Actions)
    {
        wchar_t* action_text = _wcsdup(argv[AI_Actions]);
        const wchar_t* delimiter = L",;.";
        wchar_t* buffer = nullptr;
        wchar_t* token  = wcstok(action_text, delimiter,&buffer);
        while(token)
        {
			uint32_t result = g_SyntaxTree.Find(token);
			if (result == Tree::NoIndex)
			{
                wchar_t buffer[1024];
                swprintf(buffer,1024,L"Unknown Token '%s'",token);
                MessageBox(0, buffer, pLauncher, MB_OK);
				return false;
			}

			Action a = g_SyntaxInit[result].Value;
			if (a == Action_Debug)
				g_Debug = true;
			else
				al.push_back(a);
			token = wcstok(nullptr, delimiter, &buffer);
		}
		free(action_text);
    }
    return (al.size()>0);
}

/***********************************************************************************************
* enumerate all windows and see if the search string is part of the title 
* 
* this is intentionally NOT an exact string compare.
* in fact this function needs to be improved to return the window that matches BEST the given search string
* 
************************************************************************************************/

BOOL CALLBACK FindWindowCallback(HWND hWnd, LPARAM lParam)
{
    LONG style = GetWindowLong(hWnd, GWL_STYLE);

	bool has_sysmenu = ((style & WS_SYSMENU) != 0);
	bool is_popup = ((style & WS_POPUP) != 0);
	bool is_child = ((style & WS_CHILD) != 0);
    bool is_visible = ((style & WS_VISIBLE) != 0);
	bool is_disabled = ((style & WS_DISABLED) != 0);

    if (!is_child && has_sysmenu  && !is_disabled)
    {
        wchar_t buffer[1024];
        GetWindowText(hWnd, buffer, 1024);
        if (wcsstr(buffer, g_WindowName))
        {
            g_window = hWnd;
        }
    }
    return true;
}


bool SearchWindow()
{
    EnumWindows(FindWindowCallback, 0);

    if(g_window)
        g_window_style= GetWindowLong(g_window, GWL_STYLE);

    return (g_window!=0);
}

/***********************************************************************************************
*
* go through the action list.
* the first action that can be executed will be executed, and the function will return
* 
************************************************************************************************/

void ExecuteActions(ActionList& al)
{
    for(Action action : al)
    {
        switch(action)
        {
        case Action_Open:
            if (g_window == 0)
            {
                if (g_argc >= AI_Executable)
                {
                    const wchar_t* pParamaters = (g_argc >= AI_Parameters) ? g_argv[AI_Parameters] : nullptr;
                    ShellExecute(NULL, L"open", g_argv[AI_Executable], pParamaters, NULL, SW_SHOWDEFAULT);
					if (g_Debug) MessageBox(0, L"Action Open", pLauncher, MB_OK);
                    return;
                }
                else MessageBox(0, L"Missing exe name", pLauncher, MB_OK);

            }
            break;

        case Action_Close:
            if(g_window!=0)
            {
                CloseWindow(g_window);
				if (g_Debug) MessageBox(0, L"Action Close", pLauncher, MB_OK);
                return;
            }
            break;

		case Action_Focus:
            if (ActionFocus())
            {
				if (g_Debug) MessageBox(0, L"Action Focus", pLauncher, MB_OK);
                return;
            }
			break;

		case Action_Minimize:
            if(g_window && !(g_window_style& WS_MINIMIZE))
            {
				PostMessage(g_window, WM_SYSCOMMAND, SC_MINIMIZE,0);
				if (g_Debug) MessageBox(0, L"Action Minimize", pLauncher, MB_OK);
                return;
            }
			break;

		case Action_Restore:
			if (g_window && (g_window_style & (WS_MINIMIZE| WS_MAXIMIZE)))
			{
				PostMessage(g_window, WM_SYSCOMMAND, SC_RESTORE, 0);
				ActionFocus();
				if (g_Debug) MessageBox(0, L"Action Restore", pLauncher, MB_OK);
				return;
			}
			break;

        case Action_Maximize:
			if (g_window && !(g_window_style & WS_MAXIMIZE))
			{
                PostMessage(g_window, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
				ActionFocus();
                if (g_Debug) MessageBox(0, L"Action Maximize", pLauncher, MB_OK);
                return;
			}
			break;
        }
    }
}

bool IsActiveWindow()
{
    HWND search = g_ActiveWindow;
    while (search)
    {
        if (g_window == search)
            return true;
        search = GetParent(search);
    }
    return false;
}

bool ActionFocus()
{
	if (g_window && !IsActiveWindow())
	{
		if (g_Debug) MessageBox(0, L"Action Focus", pLauncher, MB_OK);
		SetForegroundWindow(g_window);
		return true;
	}
    return false;
}

