����һ�� SMC.EXE �е���ʹ�õ����ַ����ٴ�������á�
����һ��
      pushf
      or    byte ptr [esp+1],01h
      popf
      nop

�ڶ���
      pushad
      mov    edi,offset sContext
      mov    ecx,sizeof CONTEXT
      xor    eax,eax
      cld
      rep    stosb
      invoke  GetCurrentThread
      mov    edi,eax
      mov    [sContext].ContextFlags, CONTEXT_ALL
      invoke  GetThreadContext,edi,addr sContext
      mov    [sContext].ContextFlags, CONTEXT_DEBUG_REGISTERS
      mov    [sContext].iDr7,101h ;LOCAL_EXACT_BPM_ENABLED + DR0_ENABLED
      invoke  SetThreadContext,edi,addr sContext
      popad

������д SMC ��һ��ʱϣ���ܹ���ԭ�������������Ʋ���̫���ӣ��Գ�ѧ�߶����ܹ������׿��������Է����ٵĴ��벢û����������ϸ���������á�
�����Ĵ���Ҳֻ������������С�

��ʵ��������ͨ������ֻ��SMC������ȫ�����������ԵñȽϵ��������������ĵ�������˵����������������ȱ�����Ķ��أ�

������ܵ�SMC������ȫ������ԭ��������SMC�����ηֶν��ܺ���Ҫ���еĴ��룬�����б߽��ܡ����ܴ������Կ����ͨ������֮ǰ���еĴ����ֽ��볤�ȶ��á���
  KEY = CalcKEY(INIT_KEY, PrevCode, PrevCodeLen)
  NextCode = Decrypt(NextCode, KEY)
���֮ǰ���еĴ����ֽڱ��޸ģ���������Կ����Ӷ��޷���ȷ���ܳ�����Ҫ���еĴ��롣

������ƿ��Է������� INT 3 �ϵ���и��ٵķ�ʽ��Ҳ����˵����������ͼ�ڵ�ǰ���еĴ����ĳ���� INT 3 �ϵ�ʱ���ᵼ�´����ֽڱ��޸ġ�������Ӱ����
����������Ĵ������Կ������Կ����ʱ���ܴ���õ�����һ�����߰�����ֽڣ���Ȼ�޷�������������ĸ��١�

��������֪���������� INT 3 �ϵ���и���֮�⣬������ʹ�õ������ٺ�Ӳ���������ϵ���и��١������ϵ���Ʋ����ܷ��������ַ�ʽ�ĵ��Ը��١�

�������������Ƶ�ȱ�ݣ���Ҫ�ڴ����м�����صķ����ٴ�����SMC��ϣ��������ܸ��õķ��������ߡ�������Ҫ�����Խ���SMC�Ļ�������Ϊ������û��Ϊ����ϸչ��˵�������������˵���䡣

ǰ�����ĵ�һ�η����ٴ��룬��Ե���Ҫ�ǵ������١�������û�б������߸���ʱ��ִ�е� popf ����ʱ����������쳣����������֮ǰ�Ѿ�ͨ�����´���
Block1:
      call  loc_next
      ....
loc_next:  push  fs:[0]
      mov    fs:[0],esp

�������Լ����쳣���������쳣�������׵�ַ��Ϊ call loc_next ����һ��ָ�
      ;-=-=�������쳣����������ʼ��ַ=-=-
      mov    esi,[esp+4]
      assume  esi:ptr EXCEPTION_RECORD
      mov    edi,[esp+0Ch]
      assume  edi:ptr CONTEXT
      cmp    [esi].ExceptionCode,EXCEPTION_SINGLE_STEP
      jz    @F
      mov    eax,ExceptionContinueSearch
      ret
@@:      mov    eax,[edi].regEax
      xchg  eax,[edi].regEdx
      mov    [edi].regEax,eax
      xor    eax,eax
      mov    [edi].iDr0,eax
      and    [edi].iDr1,eax
      and    [edi].iDr2,eax
      and    [edi].iDr3,eax
      and    [edi].iDr6,0FFFF0FF0h
      and    [edi].iDr7,eax
      mov    [edi].ContextFlags,CONTEXT_ALL
      mov    eax,ExceptionContinueExecution
      ret

��������������쳣ʱ���쳣�������еĴ�����Ҫ�ǽ��� EAX �� EDX �Ĵ�����ֵ��Ȼ����򷵻ص��쳣�����ִ�С�
���ｻ���Ĵ�����������ʲô�أ��������������м�����Կ��һ�δ��룺

@@:      lodsd
      xor    eax,edx
      xor    eax,ecx
      ;-=-=�ѷ��������������=-=-
      pushf
      or    byte ptr [esp+1],01h
      popf
      nop
      loop  @B

���ϴ����У�EDX ��ʼֵΪ INIT_KEY ��ECX Ϊ���볤�ȣ�EAX Ϊ���ܵĴ����ֽڡ����Կ������û���쳣���������� EAX �� EDX �Ĵ�������ôÿһ��ѭ��
ʱ����õ���ֵ�������� EAX �У�����һ��ѭ��ʱ���� lodsd ָ��ᶪʧ֮ǰ����� EAX ��ֵ�������� pushf / popf �����쳣�����쳣�������н���
�Ĵ���ֵ�������ܵ�ͬ�����µĴ��룺

@@:      lodsd
      xor    eax,edx
      xor    eax,ecx
      xchg  eax,edx
      loop  @B

��������� EDX �б���ļ��Ǽ���õ�����Կֵ��

�����ͼ����������δ��룬��������Ļ�����������ǵ������ٵ� popf ʱ���򲻻��ٲ������������쳣��ֱ�����е����� nop ָ�������û�������쳣������
��� EAX �� EDX �Ĵ���ֵҲ��û�н��������������ԿҲ���Ǵ���ġ�

�����֪�������ԭ����Ϊ������ʱ��Ȼ������ɵ���������� pushf / popf �ĵز�����ͬ�����������ԭ��Ҳ���Խ�������ķ��������ٵĴ�����SMC��������������١�

�����������ܵڶ��η����ٴ��롣��ʵ��δ���Ҳ�Ǻ����ż��뵽��������֮�У������ڴ��뿪ʼ�ͽ���ʱ���� pushad / popad �����мĴ������б���ͻָ�����Ҳ����ע�͵�
��δ�������±����£������Ը��ٶԱ�һ�¡�

��δ���Ҳ�Ǽ��ڼ�����Կ������֮�У�����������壬ͬ����Ӱ�쵽��Կ�ļ��㣬������޷����ٵ���һ������SMC���ܵõ��Ĵ��롣
��δ���ͨ��ʹ��DR0���ԼĴ�����ʹ�õ����߲����ڸ���ʱ����ʹ��Ӳ���Ĵ����ϵ��BPM�ϵ㣬һ�����¶ϵ㣬��ʹ�� OllyDBG ����ʱ���е� SetThreadContext ʱ���ܻᴥ���ϵ㣬�Ӷ��ı��˳�������̣����ս�Ӱ�����ļ������С�