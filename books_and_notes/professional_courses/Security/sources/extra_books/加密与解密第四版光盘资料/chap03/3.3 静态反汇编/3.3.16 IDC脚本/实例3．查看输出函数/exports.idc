//exports.idc 
//(c)  www.PEDIY.com 2000-2008

#include <idc.idc>

static main()
{
    auto x, ord, ea;
    Message("\n Program Entry Points: \n \n");
	for ( x=0; x<= GetEntryPointQty(); x = x+1){  //GetEntryPointQty()�õ���ڵ����
		ord =  GetEntryOrdinal( x );          //�õ�����ڵ��������
		ea = GetEntryPoint( ord );
        Message( Name( ea ) + ":  Ordinal " + ltoa( ord,16 ) + " at offset " + ltoa( ea, 16) + "\n");
	}
    Message("\n" + "Export Parsing Complete\n");
}
