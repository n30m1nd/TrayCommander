#include <windows.h>
#include <strings.h>
#include <stdio.h>
/* Define the magic for sys tray */
#define MYWM_TRAY (WM_APP+30)
#define BUFSIZE 256
#define CONFIG_FILE "n30vpn.cfg"
#define CONTEXT_MENU_MSG 900 //high number so default action doesn't take place
#define CONTEXT_MENU_MSG_EXIT 911

/*  Declare procedures  */
LRESULT CALLBACK WindowProcedure (HWND, UINT, WPARAM, LPARAM);
BOOL ShowPopupMenu( HWND hWnd, POINT *curpos, int wDefaultItem );

/*  Make the class name into a global variable  */
char szClassName[ ] = "VPNn30TrayMgr"; 
/* Data structure for tray icon */
NOTIFYICONDATA nid;      
/* File pointer */
FILE *fptr;
/* Commands for later use */
char commands[10][BUFSIZE];     /* Maximum 10 commands, 256 chars */
/* Total commands read from file */
int posOfCmd;

int WINAPI WinMain (HINSTANCE hThisInstance,
                    HINSTANCE hPrevInstance,
                    LPSTR lpszArgument,
                    int nFunsterStil){
    HWND hwnd;               /* This is the handle for our window */
    MSG messages;            /* Here messages to the application are saved */
    WNDCLASSEX wincl;        /* Data structure for the windowclass */
    char * line;             /* Each line read from file */
    size_t bytesread;
    posOfCmd = 0;


    /* The Window structure */
    wincl.hInstance = hThisInstance;
    wincl.lpszClassName = szClassName;
    wincl.lpfnWndProc = WindowProcedure;      /* This function is called by windows */
    wincl.style = CS_DBLCLKS;                 /* Catch double-clicks */
    wincl.cbSize = sizeof (WNDCLASSEX);

    /* Use default icon and mouse-pointer */
    wincl.hIcon = LoadIcon (NULL, IDI_APPLICATION);
    wincl.hIconSm = LoadIcon (NULL, IDI_APPLICATION);
    wincl.hCursor = LoadCursor (NULL, IDC_ARROW);
    wincl.lpszMenuName = NULL;                 /* No menu */
    wincl.cbClsExtra = 0;                      /* No extra bytes after the window class */
    wincl.cbWndExtra = 0;                      /* structure or the window instance */
    /* Use Windows's default color as the background of the window */
    wincl.hbrBackground = (HBRUSH) COLOR_BACKGROUND;
    /* Register the window class, and if it fails quit the program */
    if (!RegisterClassEx (&wincl))
        return 0;

    /* The class is registered, let's create the program*/
    hwnd = CreateWindowEx (
           0,                   /* Extended possibilites for variation */
           szClassName,         /* Classname */
           "n30cmd Tray",       /* Title Text */
           WS_OVERLAPPEDWINDOW, /* default window */
           CW_USEDEFAULT,       /* Windows decides the position */
           CW_USEDEFAULT,       /* where the window ends up on the screen */
           544,                 /* The programs width */
           375,                 /* and height in pixels */
           HWND_DESKTOP,        /* The window is a child-window to desktop */
           NULL,                /* No menu */
           hThisInstance,       /* Program Instance handler */
           NULL                 /* No Window Creation data */
           );
           
    /* Config file logic */
    readagain:
    posOfCmd = 0;
    if (!(fptr = fopen(CONFIG_FILE, "r"))){
       int choice = MessageBox(hwnd, "[-] Could not open config file... "\
            "Do you want to create it now?\n"\
            "Write each command in a separate line!", 
            "IO File Error", 
            MB_YESNO); 
       if (choice == IDYES){
           FILE *cfgfile;
           cfgfile = fopen(CONFIG_FILE, "w");
           fclose(cfgfile);
           ShellExecuteA(NULL, "open", "notepad.exe", CONFIG_FILE, NULL, SW_SHOW);  
           MessageBox(hwnd, "Close this box after you finish editing", "Editing...", MB_OK);
           goto readagain;
       } else return (0);
    }
    
    while (fgets(commands[posOfCmd], BUFSIZE, fptr) && (posOfCmd < 10)){ 
        commands[posOfCmd][strlen(commands[posOfCmd])] = '\0';
        posOfCmd++;
    }
    fclose(fptr);
    /* The Tray Icon structure */
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hwnd;
    nid.uID = 30;
    nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    nid.uCallbackMessage = MYWM_TRAY;
    HMODULE shell32 = LoadLibraryEx("shell32.dll", NULL, LOAD_LIBRARY_AS_DATAFILE);
    nid.hIcon = LoadIcon(shell32, MAKEINTRESOURCE(14));
    strcpy(nid.szTip, "n30 - Cmd Launcher");
    
    /* Call to create the icon */
    Shell_NotifyIcon(NIM_ADD, &nid);
    /* Make the window visible on the screen */
    //ShowWindow (hwnd, nFunsterStil);

    /* Run the message loop. It will run until GetMessage() returns 0 */
    while (GetMessage (&messages, NULL, 0, 0))
    {
        /* Translate virtual-key messages into character messages */
        TranslateMessage(&messages);
        /* Send message to WindowProcedure */
        DispatchMessage(&messages);
    }

    /* The program return-value is 0 - The value that PostQuitMessage() gave */
    return messages.wParam;
}

BOOL ShowPopupMenu( HWND hWnd, POINT *curpos, int wDefaultItem ) {
    /* Add items to context menu */
    HMENU hPop = CreatePopupMenu();
    int i;
    for(i = 0; i < posOfCmd; i++){
        InsertMenu( hPop, i, MF_BYPOSITION | MF_STRING, CONTEXT_MENU_MSG+i, commands[i]);
    }
    InsertMenu( hPop, i+1, MF_BYPOSITION | MF_STRING, CONTEXT_MENU_MSG+11, "Exit");
    
    /* Set default item */
    SetFocus          ( hWnd );
    SendMessage       ( hWnd, WM_INITMENUPOPUP, (WPARAM)hPop, 0 );
    /* Graphical showing the popup menu */
    { //Not needed but keeps mental sanity
    /* Get cursor to create context menu on cursor */
    POINT pt;
    if (!curpos) {
      GetCursorPos( &pt );
      curpos = &pt;
    }
    
    {
      /* Display the context menu */
      WORD cmd = TrackPopupMenu( hPop, TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_NONOTIFY, curpos->x, curpos->y, 0, hWnd, NULL );
      /*Send callback message to the application handle (window) */
      SendMessage( hWnd, WM_COMMAND, cmd, 0 );
    }
    }
    
    DestroyMenu(hPop);
    
    return 0;
}

/*  This function is called by the Windows function DispatchMessage()  */
LRESULT CALLBACK WindowProcedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    int msgboxoption;
    
    switch (message)                  /* handle the messages */
    {
        case WM_CLOSE:
            switch (MessageBox(hwnd, "Close application? \"No\" sends it to tray", "n30Tray", MB_YESNOCANCEL)){ 
                   case IDYES:
                        SendMessage(hwnd, WM_DESTROY, NULL, NULL);
                        break;
                   case IDNO:
                        ShowWindow (hwnd, SW_HIDE);                
                        break;
            } 
            break;
        case WM_DESTROY:
            PostQuitMessage (0);       /* send a WM_QUIT to the message queue */
            Shell_NotifyIcon(NIM_DELETE, &nid); /* Get rid of the n30tray */
            break;
        case MYWM_TRAY: //A message arrives from the tray, what could it be?
            switch (lParam){
                   case WM_LBUTTONUP: //Left Click
                       ShowWindow (hwnd, SW_RESTORE);
                       break; 
                   case WM_RBUTTONUP:
                       ShowPopupMenu(hwnd, NULL, -1);
                       break; 
                   }
            break;
        case WM_COMMAND: //A message comes from the context menu!
            switch ((int)wParam){
                   case CONTEXT_MENU_MSG_EXIT:
                       SendMessage(hwnd, WM_DESTROY, NULL, NULL);
                       break;
                   default:
                       if ((int)wParam >= CONTEXT_MENU_MSG && (int)wParam < CONTEXT_MENU_MSG+10){
                           char* localCommand = commands[(int)wParam-CONTEXT_MENU_MSG];
                           system(localCommand);
                       }
                   }
                        
            break;
        default:                      /* for messages that we don't deal with */
            return DefWindowProc (hwnd, message, wParam, lParam);
    }
    
    return 0;
}

