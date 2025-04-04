export module hu.net.impl.asio.AsioType;


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Import (AsioType)
////////////////////////////////////////////////////////////////////////////////////////////////////

import <asio.hpp>;

import hu.Util;
import hu.Type;

import "hu/Macro.hpp";


namespace hu {

////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Using (AsioType)
////////////////////////////////////////////////////////////////////////////////////////////////////

export namespace asio_impl  = asio;
export namespace asio_error = asio::error;

export using AsioErrorCode  = asio_impl::error_code;
export using AsioSocketBase = asio_impl::socket_base;
export using AsioContext    = asio_impl::io_context;
export using AsioTcp        = asio_impl::ip::tcp;
export using AsioEndPoint   = asio_impl::ip::tcp::endpoint;
export using AsioAcceptor   = asio_impl::ip::tcp::acceptor;
export using AsioResolver   = asio_impl::ip::tcp::resolver;
export using AsioSocket     = asio_impl::ip::tcp::socket;

} // hu


namespace hu::util {

////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Function (AsioType)
////////////////////////////////////////////////////////////////////////////////////////////////////

// Asio 오류 문자열을 얻는다.
export String asio_error_str(
    const AsioErrorCode& ec
)
{
    return get_format_str( _T( "ErrCode = {}, ErrMsg = {}" ),
        ec.value(), astr_to_wstr( ec.message() ) );
}

} // hu::util
