export module hu.crypt.CryptBase;


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Import (CryptBase)
////////////////////////////////////////////////////////////////////////////////////////////////////

import hu.crypt.CryptType;
import hu.pattern.INoCopy;
import hu.rand.Random;
import hu.Type;


namespace hu {

// 암호화 구현을 위한 기반 클래스
export class CryptBase : private INoCopy
{
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Constructor & Destructor (CryptBase)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    explicit CryptBase(
        const CryptConfigInfo& config
    ) :
        config_ { config }
    {
    }

    virtual ~CryptBase()
    {
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Public (CryptBase)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    // 공유키를 생성한다.
    virtual void CreateSharedKey(
        [[maybe_unused]] const Buffer& other_public_key
    )
    {
    }

    // 암호화 한다.
    virtual void Encrypt(
        [[maybe_unused]] Buffer& buffer
    ) const
    {
    }

    // 복호화 한다.
    virtual void Decrypt(
        [[maybe_unused]] Buffer& buffer
    ) const
    {
    }

    // 논스 크기를 얻는다.
    virtual Size GetNonceSize() const
    {
        return 12;
    }


public:
    // 공개키를 얻는다.
    const Buffer& GetPublicKey() const
    {
        return public_key_;
    }

    // 논스를 생성한다.
    void CreateNonce(
        const UInt32 seed
    )
    {
        nonce_.clear();

        FixRandomInt rand { seed, 0x00, 0xFF };

        const auto nonce_size = GetNonceSize();
        for ( auto i = 0; i < nonce_size; ++i )
            nonce_.push_back( static_cast<Byte>( rand.Generate() ) );
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: protected (CryptBase)
    ////////////////////////////////////////////////////////////////////////////////////////////////

protected:
    const CryptConfigInfo& config_;
    Buffer                 public_key_;
    Buffer                 nonce_;
};

} // hu
