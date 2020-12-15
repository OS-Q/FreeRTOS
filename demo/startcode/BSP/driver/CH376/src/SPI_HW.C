/* CH376оƬ Ӳ����׼SPI�������ӵ�Ӳ������� V1.0 */
/* �ṩI/O�ӿ��ӳ��� */

#include	"HAL.H"

/* �����е�Ӳ�����ӷ�ʽ����(ʵ��Ӧ�õ�·���Բ����޸��������弰�ӳ���) */
/* ��Ƭ��������    CH376оƬ������
      P1.4                 SCS
      P1.5                 SDI
      P1.6                 SDO
      P1.7                 SCK      */
#define	CH376_SPI_SCS			P14		/* �ٶ�CH376��SCS���� */
#define	CH376_SPI_SDO			P16		/* �ٶ�CH376��SDO���� */

sfr		SPDR = 0x86;	/* SPI���ݼĴ��� */
sfr		SPSR = 0xAA;	/* SPI״̬�Ĵ��� */
sfr		SPCR = 0xD5;    /* SPI���ƼĴ��� */
#define	SPI_IF_TRANS	0x80	/* SPI�ֽڴ�����ɱ�־,��SPSR��λ7 */

#define CH376_INT_WIRE			INT0	/* �ٶ�CH376��INT#����,���δ������ôҲ����ͨ����ѯ�����ж������SDO����״̬ʵ�� */

void	CH376_PORT_INIT( void )  /* ����ʹ��SPI��дʱ��,���Խ��г�ʼ�� */
{
/* �����Ӳ��SPI�ӿ�,��ô��ʹ��mode3(CPOL=1&CPHA=1)��mode0(CPOL=0&CPHA=0),CH376��ʱ�������ز�������,�½������,����λ�Ǹ�λ��ǰ */
	CH376_SPI_SCS = 1;  /* ��ֹSPIƬѡ */
/* ����˫��I/O����ģ��SPI�ӿ�,��ô�����ڴ�����SPI_SCS,SPI_SCK,SPI_SDIΪ�������,SPI_SDOΪ���뷽�� */
	SPCR = 0x5C;  /* ����SPIģʽ3, DORD=0(MSB first), CPOL=1, CPHA=1, CH376Ҳ֧��SPIģʽ0 */
}

void	mDelay0_5uS( void )  /* ������ʱ0.5uS,���ݵ�Ƭ����Ƶ���� */
{
}

UINT8	Spi376Exchange( UINT8 d )  /* Ӳ��SPI���������8��λ���� */
{  /* Ϊ������ٶ�,���Խ����ӳ������ɺ��Լ����ӳ�����ò�� */
	SPDR = d;  /* �Ƚ�����д��SPI���ݼĴ���,Ȼ���ѯSPI״̬�Ĵ����Եȴ�SPI�ֽڴ������ */
	while ( ( SPSR & SPI_IF_TRANS ) == 0 );  /* ��ѯSPI״̬�Ĵ����Եȴ�SPI�ֽڴ������ */
	SPSR &= ~ SPI_IF_TRANS;  /* ���SPI�ֽڴ�����ɱ�־,�еĵ�Ƭ�����Զ���� */
	return( SPDR );  /* �Ȳ�ѯSPI״̬�Ĵ����Եȴ�SPI�ֽڴ������,Ȼ���SPI���ݼĴ����������� */
}

#define	xEndCH376Cmd( )	{ CH376_SPI_SCS = 1; }  /* SPIƬѡ��Ч,����CH376����,������SPI�ӿڷ�ʽ */

void	xWriteCH376Cmd( UINT8 mCmd )  /* ��CH376д���� */
{
#ifdef	CH376_SPI_BZ
	UINT8	i;
#endif
	CH376_SPI_SCS = 1;  /* ��ֹ֮ǰδͨ��xEndCH376Cmd��ֹSPIƬѡ */
/* ����˫��I/O����ģ��SPI�ӿ�,��ô����ȷ���Ѿ�����SPI_SCS,SPI_SCK,SPI_SDIΪ�������,SPI_SDOΪ���뷽�� */
	CH376_SPI_SCS = 0;  /* SPIƬѡ��Ч */
	Spi376Exchange( mCmd );  /* ���������� */
#ifdef	CH376_SPI_BZ
	for ( i = 30; i != 0 && CH376_SPI_BZ; -- i );  /* SPIæ״̬��ѯ,�ȴ�CH376��æ,��������һ�е���ʱ1.5uS���� */
#else
	mDelay0_5uS( ); mDelay0_5uS( ); mDelay0_5uS( );  /* ��ʱ1.5uSȷ����д���ڴ���1.5uS,����������һ�е�״̬��ѯ���� */
#endif
}

#ifdef	FOR_LOW_SPEED_MCU  /* ����Ҫ��ʱ */
#define	xWriteCH376Data( d )	{ Spi376Exchange( d ); }  /* ��CH376д���� */
#define	xReadCH376Data( )		( Spi376Exchange( 0xFF ) )  /* ��CH376������ */
#else
void	xWriteCH376Data( UINT8 mData )  /* ��CH376д���� */
{
	Spi376Exchange( mData );
//	mDelay0_5uS( );  /* ȷ����д���ڴ���0.6uS */
}
UINT8	xReadCH376Data( void )  /* ��CH376������ */
{
//	mDelay0_5uS( );  /* ȷ����д���ڴ���0.6uS */
	return( Spi376Exchange( 0xFF ) );
}
#endif

/* ��ѯCH376�ж�(INT#�͵�ƽ) */
UINT8	Query376Interrupt( void )
{
#ifdef	CH376_INT_WIRE
	return( CH376_INT_WIRE ? FALSE : TRUE );  /* ���������CH376���ж�������ֱ�Ӳ�ѯ�ж����� */
#else
	return( CH376_SPI_SDO ? FALSE : TRUE );  /* ���δ����CH376���ж��������ѯ�����ж������SDO����״̬ */
#endif
}

UINT8	mInitCH376Host( void )  /* ��ʼ��CH376 */
{
	UINT8	res;
	CH376_PORT_INIT( );  /* �ӿ�Ӳ����ʼ�� */
	xWriteCH376Cmd( CMD11_CHECK_EXIST );  /* ���Ե�Ƭ����CH376֮���ͨѶ�ӿ� */
	xWriteCH376Data( 0x65 );
	res = xReadCH376Data( );
	xEndCH376Cmd( );
	if ( res != 0x9A ) return( ERR_USB_UNKNOWN );  /* ͨѶ�ӿڲ�����,����ԭ����:�ӿ������쳣,�����豸Ӱ��(Ƭѡ��Ψһ),���ڲ�����,һֱ�ڸ�λ,���񲻹��� */
	xWriteCH376Cmd( CMD11_SET_USB_MODE );  /* �豸USB����ģʽ */
	xWriteCH376Data( 0x06 );
	mDelayuS( 20 );
	res = xReadCH376Data( );
	xEndCH376Cmd( );
#ifndef	CH376_INT_WIRE
#ifdef	CH376_SPI_SDO
	xWriteCH376Cmd( CMD20_SET_SDO_INT );  /* ����SPI��SDO���ŵ��жϷ�ʽ */
	xWriteCH376Data( 0x16 );
	xWriteCH376Data( 0x90 );  /* SDO������SCSƬѡ��Чʱ�����ж�������� */
	xEndCH376Cmd( );
#endif
#endif
	if ( res == CMD_RET_SUCCESS ) return( USB_INT_SUCCESS );
	else return( ERR_USB_UNKNOWN );  /* ����ģʽ���� */
}