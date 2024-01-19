/***************************************************************

                Card.h             

   This file contains the interface to the manufacturer code

****************************************************************/


#ifndef  __CARD_H__
#define  __CARD_H__



int     Init( LPDRIVER_INST const pNet, P_ERR_PARAM const lpErrors );
int     TestConfig( LPDRIVER_INST const pNet, P_ERR_PARAM const lpErrors);
void    PortInput(  SPECIAL_INST_PORT* const pData);
void    PortOutput( SPECIAL_INST_PORT* const pData);
void    GetDriverStatus(const LPDRIVER_INST pNet, SPECIAL_INST_COMMAND* const pData );
void    GetDeviceStatus(const LPDRIVER_INST pNet, SPECIAL_INST_COMMAND* const pData );
void    DoPeekCommand(  const LPDRIVER_INST pNet, SPECIAL_INST_COMMAND* const pData );
void    DoPokeCommand(  const LPDRIVER_INST pNet, SPECIAL_INST_COMMAND* const pData );
void    DoCommand(      const LPDRIVER_INST pNet, SPECIAL_INST* const pData );
void	PMCPutBinary( const LPDRIVER_INST pNet, DC2_BIN_TX* const pData);
void	PMCPutBinary2Axis( const LPDRIVER_INST pNet, DC2_BIN_TX* const pData);
void	PMCPutBinary2Axis2( const LPDRIVER_INST pNet, DC2_BIN_TX* const pData);

void	PMCGetBinary(  const LPDRIVER_INST pNet, DC2_BIN_TX* const pData);
void	PMCGetAscii(  const LPDRIVER_INST pNet, SPECIAL_INST* const pData );
// void	PMCPutAscii(  const LPDRIVER_INST pNet, SPECIAL_INST* const pData );
void	PMCPutAscii(   const LPDRIVER_INST pNet,SPECIAL_INST_COMMAND* const pData);

#endif      /* __CARD_H__ */

