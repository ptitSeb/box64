#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error meh!
#endif

// Complex numbers are passed in as an address in the stack

GOW(acos, dFd)
GOW(acosf, fFf)
GOM(__acosf_finite, fFf) //%noE
GOM(__acos_finite, dFd)  //%noE
GOW(acosh, dFd)
GOW(acoshf, fFf)
GOM(__acoshf_finite, fFf) //%noE
GOM(__acosh_finite, dFd)  //%noE
#ifdef HAVE_LD80BITS
GOW(acoshl, DFD)
#else
GOW2(acoshl, KFK, acosh)
#endif
#ifdef HAVE_LD80BITS
GOW(acosl, DFD)
#else
GOW2(acosl, KFK, acos)
#endif
GOW(asin, dFd)
GOW(asinf, fFf)
GOM(__asinf_finite, fFf) //%noE
GOM(__asin_finite, dFd)  //%noE
GOW(asinh, dFd)
GOW(asinhf, fFf)
#ifdef HAVE_LD80BITS
GOW(asinhl, DFD)
#else
GOW2(asinhl, KFK, asinh)
#endif
#ifdef HAVE_LD80BITS
GOW(asinl, DFD)
#else
GOW2(asinl, KFK, asin)
#endif
GO(atan, dFd)
GOW(atan2, dFdd)
GOW(atan2f, fFff)
GOM(__atan2f_finite, fFff) //%noE
GOM(__atan2_finite, dFdd)  //%noE
// atan2l   // Weak
GOW(atanf, fFf)
GOW(atanh, dFd)
GOW(atanhf, fFf)
// __atanhf_finite
// __atanh_finite
#ifdef HAVE_LD80BITS
GOW(atanhl, DFD)
#else
GOW2(atanhl, KFK, atanh)
#endif
// atanl    // Weak
//GOW(cabs, dFdd)     // only 1 arg, but is a double complex
GOW(cabsf, fFff)    // only 1 arg, but is a float complex
// cabsl    // Weak
//GOWS(cacos, pFps)   //%% complex
GOWM(cacosf, UFs)   //%noE
//GOWS(cacosh, pFps)  //%% complex
GOWM(cacoshf, UFs)  //%noE
// cacoshl  // Weak
// cacosl   // Weak
//GOW(carg, dFdd)     // 1arg, double complex
GOW(cargf, fFff)    // 1arg, float complex
// cargl    // Weak
//GOWS(casin, pFps)   //%%  complex
GOWM(casinf, UFs)   //%noE
//GOWS(casinh, pFps)  //%%  complex
GOWM(casinhf, UFs)  //%noE
// casinhl  // Weak
// casinl   // Weak
//GOWS(catan, pFps)   //%%   complex
GOWM(catanf, UFs)   //%noE
//GOWS(catanh, pFps)  //%%   complex
GOWM(catanhf, UFs)  //%noE
// catanhl  // Weak
// catanl   // Weak
GOW(cbrt, dFd)
GOW(cbrtf, fFf)
#ifdef HAVE_LD80BITS
GOW(cbrtl, DFD)
#else
GOW2(cbrtl, KFK, cbrt)
#endif
//GOWS(ccos, pFps)    //%%   return complex
GOWM(ccosf, UFs)    //%noE return complex
//GOWS(ccosh, pFps)   //%%   return complex
GOWM(ccoshf, UFs)   //%noE return complex
// ccoshl   // Weak
// ccosl    // Weak
GOW(ceil, dFd)
GOW(ceilf, fFf)
// ceill    // Weak
//GOWS(cexp, pFps)     //%%   return complex
GOWM(cexpf, UFs)     //%noE return complex
// cexpl    // Weak
// cimag    // Weak
// cimagf   // Weak
// cimagl   // Weak
//GOS(clog, pFps)   //%% return a double complex, so ret 4
// clog10   // Weak
// __clog10
// clog10f  // Weak
// __clog10f
// clog10l  // Weak
// __clog10l
GOM(clogf, UFs)    //%noE float complex doesn't trigger the ret 4, but returns an u64!
// clogl    // Weak
// conj // Weak
// conjf    // Weak
// conjl    // Weak
GOW(copysign, dFdd)
GOW(copysignf, fFff)
// copysignl    // Weak
GOW(cos, dFd)
GOW(cosf, fFf)
GOW(cosh, dFd)
GOW(coshf, fFf)
GOM(__coshf_finite, fFf) //%noE
GOM(__cosh_finite, dFd)  //%noE
// coshl    // Weak
// cosl // Weak
//GOWS(cpow, pFpsvvvvs) //%% return complex
GOWM(cpowf, UFsvvs)   //%noE
// cpowl    // Weak
//GOS(cproj, pFps)    //%%   double complex
GOM(cprojf, UFs)    //%noE
// cprojl   // Weak
// creal    // Weak
// crealf   // Weak
// creall   // Weak
//GOWS(csin, pFps)    //%%   return complex
GOWM(csinf, UFs)    //%noE return complex
//GOWS(csinh, pFps)   //%%   return complex
GOWM(csinhf, UFs)   //%noE return complex
// csinhl   // Weak
// csinl    // Weak
//GOWS(csqrt, pFps)   //%%
GOWM(csqrtf, UFs)   //%noE
// csqrtl   // Weak
//GOWS(ctan, pFps)    //%%   return complex
GOWM(ctanf, UFs)    //%noE return complex
//GOWS(ctanh, pFps)   //%%   return complex
GOWM(ctanhf, UFs)   //%noE return complex
// ctanhl   // Weak
// ctanl    // Weak
// __cxa_finalize  // Weak
// drem // Weak
// dremf    // Weak
// dreml    // Weak
GOW(erf, dFd)
GOW(erfc, dFd)
GOW(erfcf, fFf)
#ifdef HAVE_LD80BITS
GOW(erfcl, DFD)
#else
GOW2(erfcl, KFK, erfc)
#endif
GOW(erff, fFf)
#ifdef HAVE_LD80BITS
GOW(erfl, DFD)
#else
GOW2(erfl, KFK, erf)
#endif
GOW(exp, dFd)
GOW(exp10, dFd)
GOW(exp10f, fFf)
// __exp10f_finite
// __exp10_finite
// exp10l   // Weak
GOW(exp2, dFd)
GOW(exp2f, fFf)
GOM(__exp2f_finite, fFf) //%noE
GOM(__exp2_finite, dFd)  //%noE
// exp2l    // Weak
GOW(expf, fFf)
GOM(__expf_finite, fFf) //%noE
GOM(__exp_finite, dFd)  //%noE
// expl // Weak
GOW(expm1, dFd)
GOW(expm1f, fFf)
// expm1l   // Weak
GOW(fabs, dFd)
GOW(fabsf, fFf)
// fabsl    // Weak
// fdim // Weak
// fdimf    // Weak
// fdiml    // Weak
GO(feclearexcept, iFi)
GO(fedisableexcept, iFi)
GO(feenableexcept, iFi)
GO(fegetenv, iFp)
GO(fegetexcept, iFv)
GO(fegetexceptflag, iFpi)
GO(fegetround, iFv)
GO(feholdexcept, iFp)
GO(feraiseexcept, iFi)
GO(fesetenv, iFp)
GO(fesetexceptflag, iFpi)
GO(fesetround, iFi)
GO(fetestexcept, iFi)
GO(feupdateenv, iFp)
GOW(finite, iFd)
// __finite
GOW(finitef, iFf)
GO(__finitef, iFf)
// finitel  // Weak
// __finitel
GOW(floor, dFd)
GOW(floorf, fFf)
// floorl   // Weak
GOW(fma, dFddd)
GOW(fmaf, fFfff)
// fmal // Weak
GOW(fmax, dFdd)
GOW(fmaxf, fFff)
// fmaxl    // Weak
GOW(fmin, dFdd)
GOW(fminf, fFff)
// fminl    // Weak
GOW(fmod, dFdd)
GOW(fmodf, fFff)
// __fmodf_finite
// __fmod_finite
#ifdef HAVE_LD80BITS
GOW(fmodl, DFDD)
#else
GOW2(fmodl, KFKK, fmod)
#endif
GO(__fpclassify, iFd)
GO(__fpclassifyf, iFf)
GOW(frexp, dFdp)
GOW(frexpf, fFfp)
#ifdef HAVE_LD80BITS
GOW(frexpl, DFDp)
#else
GOW2(frexpl, KFKp, frexp)
#endif
// gamma    // Weak
// gammaf   // Weak
// __gammaf_r_finite
// gammal   // Weak
// __gamma_r_finite
// __gmon_start__  // Weak
GOW(hypot, dFdd)
GOW(hypotf, fFff)
GOM(__hypotf_finite, fFff) //%noE
GOM(__hypot_finite, dFdd)  //%noE
// hypotl   // Weak
GOW(ilogb, iFd)
GOW(ilogbf, iFf)
// ilogbl   // Weak
// __issignaling
// __issignalingf
// _ITM_deregisterTMCloneTable // Weak
// _ITM_registerTMCloneTable   // Weak
GO(j0, dFd)
GO(j0f, fFf)
// __j0f_finite
// __j0_finite
// j0l
GO(j1, dFd)
GO(j1f, fFf)
// __j1f_finite
// __j1_finite
// j1l
GO(jn, dFid)
GO(jnf, fFif)
// __jnf_finite
// __jn_finite
// jnl
GOW(ldexp, dFdi)
GOW(ldexpf, fFfi)
#ifdef HAVE_LD80BITS
GOW(ldexpl, DFD)
#else
GOW2(ldexpl, KFK, ldexp)
#endif
GOW(lgamma, dFd)
GOW(lgammaf, fFf)
GOW(lgammaf_r, fFfp)
// __lgammaf_r_finite
#ifdef HAVE_LD80BITS
GOW(lgammal, DFD)
#else
GOW2(lgammal, KFK, lgamma)
#endif
#ifdef HAVE_LD80BITS
GOW(lgammal_r, DFDp)
#else
GOW2(lgammal_r, KFKp, lgamma_r)
#endif
GOW(lgamma_r, dFdp)
// __lgamma_r_finite
DATAV(_LIB_VERSION, 4)
GOW(llrint, IFd)
GOW(llrintf, IFf)
// llrintl  // Weak
GOW(llround, IFd)
GOW(llroundf, IFf)
// llroundl // Weak
GOW(log, dFd)
GOW(log10, dFd)
GOW(log10f, fFf)
GOM(__log10f_finite, fFf) //%noE
GOM(__log10_finite, dFd)  //%noE
// log10l   // Weak
GOW(log1p, dFd)
GOW(log1pf, fFf)
// log1pl   // Weak
GOW(log2, dFd)
GOW(log2f, fFf)
GOM(__log2f_finite, fFf) //%noE
GOM(__log2_finite, dFd)  //%noE
// log2l    // Weak
GOW(logb, dFd)
GOW(logbf, fFf)
// logbl    // Weak
GOW(logf, fFf)
GOM(__logf_finite, fFf) //%noE
GOM(__log_finite, dFd)  //%noE
#ifdef HAVE_LD80BITS
GOW(logl, DFD)
#else
GOW2(logl, KFK, log)
#endif
GOW(lrint, iFd)
GOW(lrintf, iFf)
// lrintl   // Weak
GOW(lround, iFd)
GOW(lroundf, iFf)
// lroundl  // Weak
// matherr  // Weak
GOW(modf, dFdp)
GOW(modff, fFfp)
// modfl    // Weak
// nan  // Weak
// nanf // Weak
// nanl // Weak
GOW(nearbyint, dFd)
GOW(nearbyintf, fFf)
// nearbyintl   // Weak
GOW(nextafter, dFdd)
GOW(nextafterf, fFff)
// nextafterl   // Weak
GOW(nexttoward, dFdD)
GOW(nexttowardf, fFfD)
// nexttowardl  // Weak
GOW(pow, dFdd)
GOW(pow10, dFd)
GOW(pow10f, fFf)
#ifdef HAVE_LD80BITS
GOWM(pow10l, DFD)   //%noE
#else
GOWM(pow10l, KFK)   //%noE
#endif
GOW(powf, fFff)
GOM(__powf_finite, fFff) //%noE
GOM(__pow_finite, dFdd)  //%noE
#ifdef HAVE_LD80BITS
GOW(powl, DFDD)
#else
GOW2(powl, KFKK, pow)
#endif
GOW(remainder, dFdd)
GOW(remainderf, fFff)
// __remainderf_finite
// __remainder_finite
// remainderl   // Weak
GOW(remquo, dFddp)
GOW(remquof, fFffp)
// remquol  // Weak
GOW(rint, dFd)
GOW(rintf, fFf)
// rintl    // Weak
GOW(round, dFd)
GOW(roundf, fFf)
// roundl   // Weak
// scalb    // Weak
// scalbf   // Weak
// __scalbf_finite
// __scalb_finite
// scalbl   // Weak
GOW(scalbln, dFdi)
GOW(scalblnf, fFfi)
// scalblnl // Weak
GOW(scalbn, dFdi)
GOW(scalbnf, fFfi)
// scalbnl  // Weak
// __signbit
// __signbitf
DATAB(signgam, 4)
// significand  // Weak
// significandf // Weak
// significandl // Weak
GOW(sin, dFd)
GOW(sincos, vFdpp)
GOW(sincosf, vFfpp)
// sincosl  // Weak
GOW(sinf, fFf)
GOW(sinh, dFd)
GOW(sinhf, fFf)
GOM(__sinhf_finite, fFf) //%noE
GOM(__sinh_finite, dFd)  //%noE
// sinhl    // Weak
// sinl // Weak
GOW(sqrt, dFd)
GOW(sqrtf, fFf)
GOM(__sqrtf_finite, fFf) //%noE
GOM(__sqrt_finite, dFd)  //%noE
// sqrtl    // Weak
GO(tan, dFd)
GOW(tanf, fFf)
GOW(tanh, dFd)
GOW(tanhf, fFf)
// tanhl    // Weak
// tanl // Weak
GOW(tgamma, dFd)
GOW(tgammaf, fFf)
#ifdef HAVE_LD80BITS
GOW(tgammal, DFD)
#else
GOW2(tgammal, KFK, tgamma)
#endif
GOW(trunc, dFd)
GOW(truncf, fFf)
// truncl   // Weak
GO(y0, dFd)
GO(y0f, fFf)
// __y0f_finite
// __y0_finite
// y0l
GO(y1, dFd)
GO(y1f, fFf)
// __y1f_finite
// __y1_finite
// y1l
GO(yn, dFid)
GO(ynf, fFif)
// __ynf_finite
// __yn_finite
// ynl
