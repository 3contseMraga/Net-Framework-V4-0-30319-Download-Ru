
/*-----------------------------------------------------------------------
��17��  �����������
 DetectingBk.cpp -- ͨ��CRC���ϵ�
  ����begindecrypt��enddecrypt֮���CRCֵ��������δ�����bpx�ϵ㣬���ᱻ����
����������ܣ����İ棩��
(c)  ��ѩѧԺ www.kanxue.com 2000-2018
-----------------------------------------------------------------------*/

#include <windows.h>

DWORD CRC32(BYTE*,DWORD);
	

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    PSTR szCmdLine, int iCmdShow)
{
	DWORD address1,address2,size;

	_asm mov address1,offset begindecrypt;
	_asm mov address2,offset enddecrypt;

	begindecrypt : //��Ǵ���������ַ
	MessageBox(NULL,TEXT ("Hello world!"),TEXT ("OK"),MB_ICONEXCLAMATION);
	enddecrypt ://��Ǵ���Ľ�����ַ

	size=address2-address1;	
	if(CRC32((BYTE*)address1,size)==0x78E888AE) //0x78E888AE��ֵ������ͨ��SoftICE�ȵ��Թ��߸��ٵõ�������������±���
		MessageBox(NULL,TEXT ("Breakpoint not found!"),TEXT ("Breakpoint detection by CRC"),MB_ICONEXCLAMATION);
	else
    		MessageBox(NULL,"Breakpoint or a change in the program found !","Breakpoint detection by CRC",MB_ICONEXCLAMATION);
	return 0;
}


////////////////////////////////////////////////////////////////
// �����ַ�����CRC32ֵ
// ������������CRC32ֵ�ַ������׵�ַ�ʹ�С
// ����ֵ: ����CRC32ֵ

DWORD CRC32(BYTE* ptr,DWORD Size)
{

	DWORD crcTable[256],crcTmp1;
	
	//��̬����CRC-32��
	for (int i=0; i<256; i++)
	 {
		crcTmp1 = i;
		for (int j=8; j>0; j--)
		 {
			if (crcTmp1&1) crcTmp1 = (crcTmp1 >> 1) ^ 0xEDB88320L;
			 else crcTmp1 >>= 1;
		}

		 crcTable[i] = crcTmp1;
	 }
	//����CRC32ֵ
	DWORD crcTmp2= 0xFFFFFFFF;
	while(Size--)
	{
		crcTmp2 = ((crcTmp2>>8) & 0x00FFFFFF) ^ crcTable[ (crcTmp2^(*ptr)) & 0xFF ];
		ptr++;
	}
		
	return (crcTmp2^0xFFFFFFFF);
}
