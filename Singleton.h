#pragma once

#include <cassert>

template < class baseClass >
class CSingleton
{

protected:

	CSingleton( void )
	{
		assert(!s_pInstance);
	}


	virtual ~CSingleton( void )
	{
		assert(s_pInstance);
	}

public:

	static bool isValidInstance( void )
	{
        //boost::mutex::scoped_lock lock( s_mutex );
		return NULL != s_pInstance;
	}

	static void setInstance( baseClass *pInstance )
	{
        CSingleton< baseClass >::release();

		s_pInstance = pInstance;
	}

	static baseClass *instance( void )
	{
		if( NULL == s_pInstance )
		{
            if( NULL == s_pInstance )
            {
                s_pInstance = new baseClass;
            }
		}

		return s_pInstance;
	}

	static void release( void )
	{
		if( NULL != s_pInstance )
		{
			delete s_pInstance;
			s_pInstance = NULL;
		}
	}

private:
static baseClass    *s_pInstance;
};

template< class baseClass >
baseClass* CSingleton< baseClass >::s_pInstance = NULL;


