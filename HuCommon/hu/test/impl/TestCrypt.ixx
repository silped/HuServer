export module hu.test.impl.TestCrypt;


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Import (TestCrypt)
////////////////////////////////////////////////////////////////////////////////////////////////////

import hu.crypt.Crypt;
import hu.crypt.CryptType;
import hu.log.Log;
import hu.net.Message;
import hu.test.TestType;
import hu.Type;

import "hu/log/LogMacro.hpp";


using namespace hu;

////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Function (TestCrypt)
////////////////////////////////////////////////////////////////////////////////////////////////////

export bool test_crypt()
{
    CryptConfigInfo config;
    config.crypt_impl = CryptImplType::kBotanPk;

    // 송신자 공개키를 생성한다.
    Crypt send_crypt { config };
    const Buffer& send_pk = send_crypt.GetPublicKey();

    // 수신자 공개키를 생성한다.
    Crypt recv_crypt { config };
    const Buffer& recv_pk = recv_crypt.GetPublicKey();

    // 교환한 공개키를 사용해서 각자 암호화 및 복호화에 사용할 키를 생성한다.
    send_crypt.CreateSharedKey( recv_pk );
    recv_crypt.CreateSharedKey( send_pk );

    // 공통 nonce를 생성한다.
    const UInt32 nonce_seed { 11111 };
    send_crypt.CreateNonce( nonce_seed );
    recv_crypt.CreateNonce( nonce_seed );

    auto send = []( const SerialInfo& req, const Crypt& send_crypt, const Crypt& recv_crypt )
    {
        // 보낼 메시지를 만들고 객체를 쓴다.
        Message req_msg;
        HU_ASSERT( req_msg.WriteObj( req ) );

        // 보낼 메시지를 암호화 한다.
        send_crypt.Encrypt( req_msg.GetBuffer() );

        // 받은 메시지를 만든다.
        Message recv_msg;
        recv_msg.Init( SerialInfo::kType, req_msg.GetBuffer() );

        // 받은 메시지를 복호화 한다.
        recv_crypt.Decrypt( recv_msg.GetBuffer() );

        // 받은 메시지로부터 객체를 읽는다.
        SerialInfo recv;
        HU_ASSERT( recv_msg.ReadObj( recv ) );

        // 받은 메시지를 검증한다.
        HU_ASSERT( req == recv );
        recv.Test();

        return true;
    };

    send( { 1, _T( "요청" ), _T( "멤버" ) }, send_crypt, recv_crypt );
    send( { 1, _T( "응답" ), _T( "멤버" ) }, recv_crypt, send_crypt );
    send( { 2, _T( "요청" ), _T( "멤버" ) }, send_crypt, recv_crypt );
    send( { 2, _T( "응답" ), _T( "멤버" ) }, recv_crypt, send_crypt );

    return true;
}
