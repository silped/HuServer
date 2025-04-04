export module hu.crypt.impl.BotanPkCrypt;


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Import (BotanPkCrypt)
////////////////////////////////////////////////////////////////////////////////////////////////////

import <botan/aead.h>;
import <botan/auto_rng.h>;
import <botan/dh.h>;
import <botan/dl_group.h>;
import <botan/pubkey.h>;
import <botan/symkey.h>;

import hu.crypt.CryptBase;


namespace hu {

// Botan을 사용해서 공개키 방식 암호화를 구현한 클래스
export class BotanPkCrypt final : public CryptBase
{
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Constructor & Destructor (BotanPkCrypt)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    explicit BotanPkCrypt(
        const CryptConfigInfo& config
    ) :
        CryptBase    { config },
        rng_         {},
        private_key_ { rng_, Botan::DL_Group( "modp/ietf/1024" ) },
        ka_          { private_key_, rng_, "KDF2(SHA-256)" },
        enc_         { Botan::AEAD_Mode::create( "AES-256/GCM", Botan::Cipher_Dir::Encryption ) },
        dec_         { Botan::AEAD_Mode::create( "AES-256/GCM", Botan::Cipher_Dir::Decryption ) }
    {
        public_key_ = private_key_.public_value();
    }

    ~BotanPkCrypt()
    {
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Public (BotanPkCrypt)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    virtual void CreateSharedKey(
        const Buffer& other_public_key
    ) override
    {
        shared_key_ = ka_.derive_key( 32, other_public_key );

        enc_->set_key( shared_key_ );
        dec_->set_key( shared_key_ );
    }

    virtual void Encrypt(
        Buffer& buffer
    ) const override
    {
        if ( shared_key_.empty() || nonce_.empty() )
            return;

        enc_->start( nonce_ );
        enc_->finish( buffer );
    }

    virtual void Decrypt(
        Buffer& buffer
    ) const override
    {
        if ( shared_key_.empty() || nonce_.empty() )
            return;

        dec_->start( nonce_ );
        dec_->finish( buffer );
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Private (BotanPkCrypt)
    ////////////////////////////////////////////////////////////////////////////////////////////////

private:
    Botan::AutoSeeded_RNG             rng_;
    Botan::DH_PrivateKey              private_key_;
    Botan::PK_Key_Agreement           ka_;
    Botan::SymmetricKey               shared_key_;
    std::unique_ptr<Botan::AEAD_Mode> enc_;
    std::unique_ptr<Botan::AEAD_Mode> dec_;
};

} // hu
