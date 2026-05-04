/*
  T.Morita
  $Id: rand.h,v 1.4 2002/11/23 11:50:59 Yoshizawa1 Exp $
 */
#ifndef __LIBUTL_RAND_H__
#define __LIBUTL_RAND_H__

#ifndef ULONG_MAX
#define ULONG_MAX 0xffffffff
#endif

/* 0 <= irnd() <= ULONG_MAX */
#ifdef KP_XBOX
static inline int irnd()
{
    extern unsigned int random_seed ;
    return ( int )( (random_seed = random_seed*1566083941UL + 1) & 0x7FFFFFFF );
}
/* 0.0f <= rnd() < 1.0f*/
static inline float rnd()
{
    return ( irnd() /(0x7FFFFFFFL+1.0f) ) ;
}

/* -1.0f < frnd() < 1.0f*/
static inline float frnd()
{
    return ( irnd() /(0x7FFFFFFF/2+1.0f)-1.0f ) ;
}
#ifdef KP_WINDOWS
static inline unsigned int m_get_rndseed()
{
    extern unsigned int random_seed ;
    return random_seed ;
}
static inline void m_set_rndseed(unsigned int seed)
{
    extern unsigned int random_seed ;
    random_seed = seed ;
}
#define	m_irnd(seed_)	((int)(((seed_) = (seed_)*1566083941UL + 1) & 0x7FFFFFFF))
#define	m_rnd(seed_)	(((float)m_irnd(seed_))/((float)(0x7FFFFFFFL+1.0f)))
#define	m_frnd(seed_)	(((float)m_irnd(seed_))/((float)(0x7FFFFFFFL/2+1.0f))-1.0f)

#endif // KP_WINDOWS
#else
static inline unsigned int irnd()
{
    extern unsigned int random_seed ;
    return (random_seed = random_seed*1566083941UL + 1) ;
}

/* 0.0f <= rnd() < 1.0f*/
static inline float rnd()
{
    return ( irnd() /(ULONG_MAX+1.0f) ) ;
}

/* -1.0f < frnd() < 1.0f*/
static inline float frnd()
{
    return ( irnd() /(ULONG_MAX/2+1.0f)-1.0f ) ;
}

#endif





/* 0 <= irnd() <= ULONG_MAX */
static inline unsigned int GM_IRnd( int *random_seed )
{
    return ((*random_seed) = (*random_seed)*1566083941UL + 1) ;
}

/* 0.0f <= rnd() < 1.0f*/
static inline float GM_Rnd( int *random_seed )
{
    return ( GM_IRnd( random_seed ) /(ULONG_MAX+1.0f) ) ;
}

static inline float GM_FRnd( int *random_seed )
{
    return ( GM_IRnd( random_seed ) /(ULONG_MAX/2+1.0f)-1.0f ) ;
}

#endif
