export module hu.crypt.Crypt;


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Import (Crypt)
////////////////////////////////////////////////////////////////////////////////////////////////////

import <memory>;

import hu.crypt.CryptBase;
import hu.crypt.CryptType;
import hu.crypt.impl.BotanPkCrypt;
import hu.pattern.INoCopy;
import hu.Type;


namespace hu {

// 암호화를 구현한 클래스
export class Crypt final : private INoCopy
{
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Constructor & Destructor (Crypt)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    Crypt() :
        config_ {},
        impl_   { create_impl( config_ ) }
    {
    }

    explicit Crypt(
        const CryptConfigInfo& config
    ) :
        config_ { config },
        impl_   { create_impl( config_ ) }
    {
    }

    ~Crypt()
    {
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Public (Crypt)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    // 공유키를 생성한다.
    void CreateSharedKey(
        const Buffer& other_public_key
    )
    {
        impl_->CreateSharedKey( other_public_key );
    }

    // 암호화 한다.
    void Encrypt(
        Buffer& buffer
    ) const
    {
        impl_->Encrypt( buffer );
    }

    // 복호화 한다.
    void Decrypt(
        Buffer& buffer
    ) const
    {
        impl_->Decrypt( buffer );
    }

    // 공개키를 얻는다.
    const Buffer& GetPublicKey() const
    {
        return impl_->GetPublicKey();
    }

    // 논스를 생성한다.
    void CreateNonce(
        const UInt32 seed
    )
    {
        impl_->CreateNonce( seed );
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: protected (Crypt)
    ////////////////////////////////////////////////////////////////////////////////////////////////

private:
    using Impl = std::unique_ptr<CryptBase>;

private:
    static Impl create_impl(
        const CryptConfigInfo& config
    )
    {
        switch ( config.crypt_impl )
        {
        case CryptImplType::kBotanPk:
            return std::make_unique<BotanPkCrypt>( config );
        }

        return std::make_unique<CryptBase>( config );
    }

private:
    CryptConfigInfo config_;
    Impl            impl_;
};

} // hu
