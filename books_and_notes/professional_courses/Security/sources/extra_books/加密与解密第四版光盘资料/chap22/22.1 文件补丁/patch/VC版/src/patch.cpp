/*-----------------------------------------------------------------------
��22��  ��������
����������ܣ����İ棩��
(c)  ��ѩѧԺ www.kanxue.com 2000-2018
-----------------------------------------------------------------------*/

#include "resource.h"
#include "peFile.c"
#include <windows.h>


//�Զ��������ڣ����Ҫ�Ż����Ƽ�ʹ����� 
//���ʹ�� VS.NET��Ҳ�����������ã��ڡ������������������ѡ����Ŀ���һ���ѡ�����ԡ�->����������->���߼���->����ڵ㡱 
//VC 6����Ҳ�����Ƶ����ã������岻�ǵ��� 
//��Ŀ���Ե��������ÿ�����ϸ�����������Ż�Ҳ��������������� 
//#pragma comment(linker, "/ENTRY:EntryPoint") 

//�����ǵ����ζ��룬Ĭ���� 4K���� Win98 ���ܸ����װ�� PE �ļ����������� PE �ļ��Ĵ�С 
//������һ������������˵�ǡ���Ч��ָ�ʲô�ģ���������Ҳע�͵��ˣ�Ҳ���Ƕ�̫С��ԭ��ɡ� 
//ֱ���ڹ���ѡ���������ã���Ŀ�����ԡ�->����������->���Ż���->��Windows98 �Ż���->���� (/OPT:NOWIN98)�� 
//#pragma comment(linker, "/OPT:NOWIN98") 


//////////////////////////////////////////////////////////////////////////////////////////////////// 

//////////////////////////////////////////////////////////////////////////////////////////////////// 
//��ں��� 
//ʹ�������Լ�����ں�����������������Ĭ���ṩ��һ��ѳ����ʼ�������Ĵ��� 
//Ϊ����һ����ͨ�� Win32SDK ��������ʹ�����ַ���������ĺ��������� WinMain() ��������������Ӧ�Ĳ��� 

/*
void EntryPoint() 
{ 
      ExitProcess(WinMain(GetModuleHandle(NULL), NULL, NULL, SW_SHOWNORMAL)); 
}
*/
//////////////////////////////////////////////////////////////////////////////////////////////////// 


HINSTANCE hInst ;
TCHAR szFileName[30], szTitleName[30] ;
HINSTANCE dllhander;

BOOL CALLBACK MainDlg(HWND, UINT, WPARAM, LPARAM);

BOOL Patch();


int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    PSTR szCmdLine, int iCmdShow)
{
	hInst = hInstance;
	DialogBoxParam (hInstance, MAKEINTRESOURCE(IDD_MAINDLG), NULL, MainDlg, NULL);
	return 0;	
}

BOOL CALLBACK MainDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) 
	{
	    case WM_INITDIALOG:
			PopFileInitialize (hDlg) ;	 
			SendMessage(hDlg,WM_SETICON,ICON_BIG,LPARAM(LoadIcon(hInst,MAKEINTRESOURCE(IDI_ICON)))); //����ͼ��
			break; 
		
        //close the dialog
        case WM_CLOSE:
            EndDialog(hDlg, NULL); //end modal dialog with EndDialog
		                           //and it has not WM_DESTORY message 
		    break;
			

		case WM_COMMAND:
			switch (LOWORD (wParam))
			{
			case IDOK:
				PopFileOpenDlg (hDlg, szFileName, szTitleName);
				EnableWindow(GetDlgItem(hDlg,IDOK2),TRUE);
				SendMessage(GetDlgItem(hDlg,IDC_ts), WM_SETTEXT, 0, (LPARAM)szFileName);
				break;
			case IDPATCH:				
				if(Patch())
					MessageBox(NULL,"�������!","Patch",MB_ICONINFORMATION);
				else
					MessageBox(NULL,"����ʧ��!","ERROR",0);

				break;

			case IDCANCEL:
				EndDialog(hDlg,NULL);
				break;
			}
			break;

		default:
			return FALSE;
	}
	
	return true;
}     


BOOL Patch()
{
	HANDLE hFile;
	DWORD szTemp;
	DWORD FileSize;
	DWORD lFileSize=40960;                //�ļ���С
	DWORD lFileOffset=0x11E3;                //��Ҫ�޸ĵ��ļ�ƫ�Ƶ�ַ
	DWORD lChanges=2;                  //��Ҫ�޲����ֽ���
	BYTE BytesToWrite[]={0x6A,00};        //д������
	
	
	hFile = CreateFile( szFileName,	
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	
	if( hFile != INVALID_HANDLE_VALUE )
	{
		FileSize=GetFileSize(hFile,&szTemp);  // ��ȡ�ļ���С
		if (FileSize == 0xFFFFFFFF) {
			CloseHandle(hFile);
			return FALSE;
		}
		if(FileSize!=lFileSize){
			CloseHandle(hFile);
			return FALSE;
		}
		
		SetFilePointer(hFile,lFileOffset,NULL,FILE_BEGIN);//�����ļ�ָ��
		if(!WriteFile(hFile,&BytesToWrite,lChanges,&szTemp,NULL))
		{
			CloseHandle(hFile);
			return FALSE;
		}
				
		CloseHandle(hFile);		
		return TRUE;
	}
	return FALSE;
	
}