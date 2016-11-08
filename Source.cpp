#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include <windows.h>
#include <tchar.h>

TCHAR szClassName[] = TEXT("Window");

class calc
{
public:
	calc(LPTSTR lpszCalc, double *ans)
	{
		g_pbuf = lpszCalc;
		g_back = TK_NONE;
		*ans = expr();
	}
private:
	enum TOKEN {
		TK_NONE = -1,
		TK_EOF = 0,
		TK_CONST,
		TK_PLUS,
		TK_MINUS,
		TK_MULT,
		TK_DIV,
		TK_LPAREN,
		TK_RPAREN
	};
	LPTSTR g_pbuf;
	double g_value;
	TOKEN g_back;
	double expr()
	{
		double val = term();
		for (;;)
		{
			const TOKEN token = get_token();
			switch (token)
			{
			case TK_PLUS:
				val += term();
				break;
			case TK_MINUS:
				val -= term();
				break;
			default:
				g_back = token;
				return val;
			}
		}
	}
	TOKEN get_token()
	{
		if (g_back >= 0)
		{
			const TOKEN ret = g_back;
			g_back = TK_NONE;
			return ret;
		}
		while (isspace(*g_pbuf))g_pbuf++;
		switch (*g_pbuf)
		{
		case '+':
			g_pbuf++;
			return TK_PLUS;
		case '-':
			g_pbuf++;
			return TK_MINUS;
		case '*':
			g_pbuf++;
			return TK_MULT;
		case '/':
			g_pbuf++;
			return TK_DIV;
		case '(':
			g_pbuf++;
			return TK_LPAREN;
		case ')':
			g_pbuf++;
			return TK_RPAREN;
		default:
			g_value = _tcstod(g_pbuf, &g_pbuf);
			return TK_CONST;
		}
	}
	double prim()
	{
		const int token = get_token();
		switch (token)
		{
		case TK_CONST:
			return g_value;
		case TK_MINUS:
			return -prim();
		case TK_LPAREN:
			{
				const double val = expr();
				get_token();
				return val;
			}
		default:
			return 0;
		}
	}
	double term()
	{
		double val1 = prim(), val2;
		g_back = get_token();
		for (;;)
		{
			const TOKEN token = get_token();
			switch (token)
			{
			case TK_MULT:
				val1 *= prim();
				break;
			case TK_DIV:
				val2 = prim();
				g_back = get_token();
				val1 /= val2;
				break;
			default:
				g_back = token;
				return val1;
			}
		}
	}
};

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HWND hEdit;
	static HWND hButton;
	switch (msg)
	{
	case WM_CREATE:
		hEdit = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), 0, WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL, 0, 0, 0, 0, hWnd, 0, ((LPCREATESTRUCT)lParam)->hInstance, 0);
		hButton = CreateWindow(TEXT("BUTTON"), TEXT("計算"), WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hWnd, (HMENU)IDOK, ((LPCREATESTRUCT)lParam)->hInstance, 0);
		break;
	case WM_SIZE:
		MoveWindow(hEdit, 10, 10, LOWORD(lParam) - 20, 32, TRUE);
		MoveWindow(hButton, 10, 50, 64, 32, TRUE);
		break;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			const int nSize = GetWindowTextLength(hEdit);
			if (nSize)
			{
				LPTSTR lpszText = (LPTSTR)GlobalAlloc(0, sizeof(TCHAR)*(nSize + 1));
				if (lpszText)
				{
					GetWindowText(hEdit, lpszText, nSize + 1);
					double ans;
					calc(lpszText, &ans);
					TCHAR szText[256];
					_stprintf_s(szText, nSize + 1, TEXT("%g"), ans);
					SetWindowText(hEdit, szText);
					SendMessage(hEdit, EM_SETSEL, 0, -1);
					GlobalFree(lpszText);
				}
			}
		}
		break;
	case WM_CLOSE:
		DestroyWindow(hWnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefDlgProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPreInst, LPSTR pCmdLine, int nCmdShow)
{
	MSG msg;
	WNDCLASS wndclass = {
		CS_HREDRAW | CS_VREDRAW,
		WndProc,
		0,
		DLGWINDOWEXTRA,
		hInstance,
		0,
		LoadCursor(0,IDC_ARROW),
		0,
		0,
		szClassName
	};
	RegisterClass(&wndclass);
	HWND hWnd = CreateWindow(
		szClassName,
		TEXT("エディットボックスに入力された計算式を計算する"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		0,
		CW_USEDEFAULT,
		0,
		0,
		0,
		hInstance,
		0
	);
	ShowWindow(hWnd, SW_SHOWDEFAULT);
	UpdateWindow(hWnd);
	while (GetMessage(&msg, 0, 0, 0))
	{
		if (!IsDialogMessage(hWnd, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return (int)msg.wParam;
}
