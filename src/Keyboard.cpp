#include <input/Keyboard.hpp>

#include <cstring> // for std::memset

using namespace input;

// Keyboard state is exactly 32 bytes.
static_assert( sizeof( Keyboard::State ) == ( 256 / 8 ) );

bool Keyboard::State::isKeyDown( Key key ) const noexcept
{
    auto k = static_cast<uint8_t>( key );
    if ( k <= 0xfe )
    {
        const auto         ptr = reinterpret_cast<const uint32_t*>( this );
        const unsigned int bf  = 1u << ( k & 0x1f );
        return ( ptr[( k >> 5 )] & bf ) != 0;
    }
    return false;
}

bool Keyboard::State::isKeyUp( Key key ) const noexcept
{
    auto k = static_cast<uint8_t>( key );
    if ( k <= 0xfe )
    {
        const auto         ptr = reinterpret_cast<const uint32_t*>( this );
        const unsigned int bf  = 1u << ( k & 0x1f );
        return ( ptr[( k >> 5 )] & bf ) == 0;
    }
    return false;
}

void KeyboardStateTracker::update( const Keyboard::State& state )
{
    auto currPtr     = reinterpret_cast<const uint32_t*>( &state );
    auto prevPtr     = reinterpret_cast<const uint32_t*>( &lastState );
    auto releasedPtr = reinterpret_cast<uint32_t*>( &released );
    auto pressedPtr  = reinterpret_cast<uint32_t*>( &pressed );
    for ( size_t j = 0; j < ( 256 / 32 ); ++j )
    {
        *pressedPtr  = *currPtr & ~( *prevPtr );
        *releasedPtr = ~( *currPtr ) & *prevPtr;

        ++currPtr;
        ++prevPtr;
        ++releasedPtr;
        ++pressedPtr;
    }

    lastState = state;
}

void KeyboardStateTracker::reset()
{
    std::memset( this, 0, sizeof( KeyboardStateTracker ) );
}

