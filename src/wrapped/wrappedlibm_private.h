#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error meh!
#endif

GOW(acos, dFd)
GOW(acosf, fFf)
GOM(__acosf_finite, fFf)
GOM(__acos_finite, dFd)
GOW(acosh, dFd)
GOW(acoshf, fFf)
GOM(__acoshf_finite, fFf)
GOM(__acosh_finite, dFd)
#ifdef HAVE_LD80BITS
GOW(acoshl, DFD)
#else
GO2(acoshl, KFK, acosh)
#endif
#ifdef HAVE_LD80BITS
GOW(acosl, DFD)
#else
GO2(acosl, KFK, acos)
#endif
GOW(asin, dFd)
GOW(asinf, fFf)
GOM(__asinf_finite, fFf)
GOM(__asin_finite, dFd)
GOW(asinh, dFd)
GOW(asinhf, fFf)
#ifdef HAVE_LD80BITS
GOW(asinhl, DFD)
#else
GO2(asinhl, KFK, asinh)
#endif
#ifdef HAVE_LD80BITS
GOW(asinl, DFD)
#else
GO2(asinl, KFK, asin)
#endif
GO(atan, dFd)
GOW(atan2, dFdd)
GOW(atan2f, fFff)
GOM(__atan2f_finite, fFff)
GOM(__atan2_finite, dFdd)
#ifdef HAVE_LD80BITS
GOW(atan2l, lFDD)
#else
GO2(atan2l, lFKK, atan2)
#endif
GOW(atanf, fFf)
GOW(atanh, dFd)
GOW(atanhf, fFf)
// __atanhf_finite
// __atanh_finite
#ifdef HAVE_LD80BITS
GOW(atanhl, DFD)
GOW(atanl, DFD)
#else
GO2(atanhl, KFK, atanh)
GO2(atanl, KFK, atan)
#endif
GOW(cabs, XFX)
GOW(cabsf, xFx)
#ifdef HAVE_LD80BITS
GOW(cabsl, DFY)
#else
GO2(cabsl, KFy, cabs)
#endif
GOW(cacos, XFX)
GOW(cacosf, xFx)
GOW(cacosh, XFX)
GOW(cacoshf, xFx)
// cacoshl  // Weak
// cacosl   // Weak
GOW(carg, XFX)
GOW(cargf, xFx)
// cargl    // Weak
GOW(casin, XFX)
GOW(casinf, xFx)
GOW(casinh, XFX)
GOW(casinhf, xFx)
// casinhl  // Weak
// casinl   // Weak
GOW(catan, XFX)
GOW(catanf, xFx)
GOW(catanh, XFX)
GOW(catanhf, xFx)
// catanhl  // Weak
// catanl   // Weak
GOW(cbrt, dFd)
GOW(cbrtf, fFf)
#ifdef HAVE_LD80BITS
GOW(cbrtl, DFD)
#else
GO2(cbrtl, KFK, cbrt)
#endif
GOW(ccos, XFX)
GOW(ccosf, xFx)
GOW(ccosh, XFX)
GOW(ccoshf, xFx)
#ifdef HAVE_LD80BITS
GOW(ccoshl, YFY)
GOW(ccosl, YFY)
#else
GO2(ccoshl, yFy, ccosh)
GO2(ccosl, yFy, ccos)
#endif
GOW(ceil, dFd)
GOW(ceilf, fFf)
#ifdef HAVE_LD80BITS
GOW(ceill, DFD)    // Weak
#else
GO2(ceill, KFK, ceil)
#endif
GOW(cexp, XFX)
GOW(cexpf, xFx)
#ifdef HAVE_LD80BITS
GOW(cexpl, YFY)
#else
GO2(cexpl, yFy, cexp)
#endif
// cimag    // Weak
// cimagf   // Weak
// cimagl   // Weak
GOW(clog, XFX)
// clog10   // Weak
// __clog10
// clog10f  // Weak
// __clog10f
// clog10l  // Weak
// __clog10l
GOW(clogf, xFx)
#ifdef HAVE_LD80BITS
GOW(clogl, YFY)
#else
GO2(clogl, yFy, clog)
#endif
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
GOM(__coshf_finite, fFf)
GOM(__cosh_finite, dFd)
#ifdef HAVE_LD80BITS
GOW(coshl, DFD)
GOW(cosl, DFD)
#else
GO2(coshl, KFK, cosh)
GO2(cosl, KFK, cos)
#endif
GOW(cpow, XFXX)
GOW(cpowf, xFxx)
#ifdef HAVE_LD80BITS
GOW(cpowl, YFYY)
#else
GO2(cpowl, yFyy, cpow)
#endif
GOW(cproj, XFX)
GOW(cprojf, xFx)
// cprojl   // Weak
// creal    // Weak
// crealf   // Weak
// creall   // Weak
GOW(csin, XFX)
GOW(csinf, xFx)
GOW(csinh, XFX)
GOW(csinhf, xFx)
#ifdef HAVE_LD80BITS
GOW(csinhl, YFY)
GOW(csinl, YFY)
#else
GO2(csinhl, yFy, csinh)
GO2(csinl, yFy, csin)
#endif
GOW(csqrt, XFX)
GOW(csqrtf, xFx)
#ifdef HAVE_LD80BITS
GOW(csqrtl, YFY)
#else
GO2(csqrtl, yFy, csqrt)
#endif
GOW(ctan, XFX)
GOW(ctanf, xFx)
GOW(ctanh, XFX)
GOW(ctanhf, xFx)
#ifdef HAVE_LD80BITS
GOW(ctanhl, YFY)
GOW(ctanl, YFY)
#else
GO2(ctanhl, yFy, ctanh)
GO2(ctanl, yFy, ctan)
#endif
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
GO2(erfcl, KFK, erfc)
#endif
GOW(erff, fFf)
#ifdef HAVE_LD80BITS
GOW(erfl, DFD)
#else
GO2(erfl, KFK, erf)
#endif
GOW(exp, dFd)
GOW(exp10, dFd)
GOW(exp10f, fFf)
// __exp10f_finite
// __exp10_finite
// exp10l   // Weak
GOW(exp2, dFd)
GOW(exp2f, fFf)
GOM(__exp2f_finite, fFf)
GOM(__exp2_finite, dFd)
#ifdef HAVE_LD80BITS
GOW(exp2l, DFD)
#else
GO2(exp2l, KFK, exp2)
#endif
GOW(expf, fFf)
GOM(__expf_finite, fFf)
GOM(__exp_finite, dFd)
#ifdef HAVE_LD80BITS
GOW(expl, DFD)
#else
GO2(expl, KFK, exp)
#endif
GOW(expm1, dFd)
GOW(expm1f, fFf)
#ifdef HAVE_LD80BITS
GOW(expm1l, DFD)
#else
GO2(expm1l, KFK, expm1)
#endif
GOW(fabs, dFd)
GOW(fabsf, fFf)
// fabsl    // Weak
GOW(fdim, dFdd)
GOW(fdimf, fFff)
// fdiml    // Weak
GO(feclearexcept, iFi)
GO(fedisableexcept, iFi)
GO(feenableexcept, iFi)
GO(fegetenv, iFp)
GO(fegetexcept, iFv)
GO(fegetexceptflag, iFpi)
GOM(fegetround, iFEv)
GO(feholdexcept, iFp)
GO(feraiseexcept, iFi)
GO(fesetenv, iFp)
GO(fesetexceptflag, iFpi)
GOM(fesetround, iFEi)
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
#ifdef HAVE_LD80BITS
GOW(floorl, DFD)
#else
GO2(floorl, KFK, floor)
#endif
GOW(fma, dFddd)
GOW(fmaf, fFfff)
#ifdef HAVE_LD80BITS
GOW(fmal, DFDDD)
#else
GO2(fmal, KFKKK, fma)
#endif
GOW(fmax, dFdd)
GOW(fmaxf, fFff)
// fmaxl    // Weak
GOW(fmin, dFdd)
GOW(fminf, fFff)
// fminl    // Weak
GOW(fmod, dFdd)
GOW(fmodf, fFff)
GOM(__fmodf_finite, fFff)
GOM(__fmod_finite, dFdd)
#ifdef HAVE_LD80BITS
GOW(fmodl, DFDD)
#else
GO2(fmodl, KFKK, fmod)
#endif
GO(__fpclassify, iFd)
GO(__fpclassifyf, iFf)
GOW(frexp, dFdp)
GOW(frexpf, fFfp)
#ifdef HAVE_LD80BITS
GOW(frexpl, DFDp)
#else
GO2(frexpl, KFKp, frexp)
#endif
// gamma    // Weak
// gammaf   // Weak
// __gammaf_r_finite
// gammal   // Weak
// __gamma_r_finite
// __gmon_start__  // Weak
GOW(hypot, dFdd)
GOW(hypotf, fFff)
GOM(__hypotf_finite, fFff)
GOM(__hypot_finite, dFdd)
#ifdef HAVE_LD80BITS
GOW(hypotl, DFDD)
#else
GO2(hypotl, KFKK, hypot)
#endif
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
#ifdef HAVE_LD80BITS
GO(jnl, DFiD)
#else
GO2(jnl, KFiK, jn)
#endif
GOW(ldexp, dFdi)
GOW(ldexpf, fFfi)
#ifdef HAVE_LD80BITS
GOW(ldexpl, DFD)
#else
GO2(ldexpl, KFK, ldexp)
#endif
GOW(lgamma, dFd)
GOW(lgammaf, fFf)
GOW(lgammaf_r, fFfp)
// __lgammaf_r_finite
#ifdef HAVE_LD80BITS
GOW(lgammal, DFD)
#else
GO2(lgammal, KFK, lgamma)
#endif
#ifdef HAVE_LD80BITS
GOW(lgammal_r, DFDp)
#else
GO2(lgammal_r, KFKp, lgamma_r)
#endif
GOW(lgamma_r, dFdp)
// __lgamma_r_finite
#ifdef STATICBUILD
//DATAV(_LIB_VERSION, 8)
#else
DATAV(_LIB_VERSION, 8)
#endif
GOWM(llrint, IFEd)
GOWM(llrintf, IFEf)
#ifdef HAVE_LD80BITS
GOWM(llrintl, IFED)
#else
GOWM(llrintl, IFEK)
#endif
GOW(llround, IFd)
GOW(llroundf, IFf)
// llroundl // Weak
GOW(log, dFd)
GOW(log10, dFd)
GOW(log10f, fFf)
GOM(__log10f_finite, fFf)
GOM(__log10_finite, dFd)
#ifdef HAVE_LD80BITS
GOW(log10l, DFD)
#else
GO2(log10l, KFK, log10) //Weak
#endif
GOW(log1p, dFd)
GOW(log1pf, fFf)
#ifdef HAVE_LD80BITS
GOW(log1pl, DFD)
#else
GO2(log1pl, KFK, log1p)
#endif
GOW(log2, dFd)
GOW(log2f, fFf)
GOM(__log2f_finite, fFf)
GOM(__log2_finite, dFd)
#ifdef HAVE_LD80BITS
GOW(log2l, DFD)
#else
GO2(log2l, KFK, log2)
#endif
GOW(logb, dFd)
GOW(logbf, fFf)
// logbl    // Weak
GOW(logf, fFf)
GOM(__logf_finite, fFf)
GOM(__log_finite, dFd)
#ifdef HAVE_LD80BITS
GOW(logl, DFD)
#else
GO2(logl, KFK, log)
#endif
GOWM(lrint, iFEd)
GOWM(lrintf, iFEf)
// lrintl   // Weak
GOW(lround, iFd)
GOW(lroundf, iFf)
#ifdef HAVE_LD80BITS
GOW(lroundl, lFD)
#else
GO2(lroundl, lFK, lround)
#endif
// matherr  // Weak
GOW(modf, dFdp)
GOW(modff, fFfp)
// modfl    // Weak
GOW(nan, dFp)
GOW(nanf, fFp)
// nanl // Weak
GOWM(nearbyint, dFEd)
GOWM(nearbyintf, fFEf)
// nearbyintl   // Weak
GOW(nextafter, dFdd)
GOW(nextafterf, fFff)
#ifdef HAVE_LD80BITS
GOW(nextafterl, DFDD)
#else
GO2(nextafterl, KFKK, nextafter)
#endif
GOW(nexttoward, dFdD)
GOW(nexttowardf, fFfD)
// nexttowardl  // Weak
GOW(pow, dFdd)
GOWM(pow10, dFd)
GOWM(pow10f, fFf)
#ifdef HAVE_LD80BITS
GOWM(pow10l, DFD)
#else
GOWM(pow10l, KFK)
#endif
GOW(powf, fFff)
GOM(__powf_finite, fFff)
GOM(__pow_finite, dFdd)
#ifdef HAVE_LD80BITS
GOW(powl, DFDD)
#else
GO2(powl, KFKK, pow)
#endif
GOW(remainder, dFdd)
GOW(remainderf, fFff)
// __remainderf_finite
// __remainder_finite
// remainderl   // Weak
GOW(remquo, dFddp)
GOW(remquof, fFffp)
// remquol  // Weak
GOWM(rint, dFEd)
GOWM(rintf, fFEf)
// rintl    // Weak
GOW(round, dFd)
GOW(roundf, fFf)
GO(roundeven, dFd)  //since C23
GO(roundevenf, fFf)
//GO(roundevenl, DFD)
#ifdef HAVE_LD80BITS
GOW(roundl, DFD)
#else
GO2(roundl, KFK, round)
#endif
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
DATAB(signgam, 8)
// significand  // Weak
// significandf // Weak
// significandl // Weak
GOW(sin, dFd)
GOW(sincos, vFdpp)
GOW(sincosf, vFfpp)
#ifdef HAVE_LD80BITS
GOW(sincosl, vFDpp)
#else
GO2(sincosl, vFKpp, sincos)
#endif
GOW(sinf, fFf)
GOW(sinh, dFd)
GOW(sinhf, fFf)
GOM(__sinhf_finite, fFf)
GOM(__sinh_finite, dFd)
#ifdef HAVE_LD80BITS
GOW(sinhl, DFD)
GOW(sinl, DFD)
#else
GO2(sinhl, KFK, sinh)
GO2(sinl, KFK, sin)
#endif
GOW(sqrt, dFd)
GOW(sqrtf, fFf)
GOM(__sqrtf_finite, fFf)
GOM(__sqrt_finite, dFd)
#ifdef HAVE_LD80BITS
GOW(sqrtl, DFD)
#else
GO2(sqrtl, KFK, sqrt)    // Weak
#endif
GO(tan, dFd)
GOW(tanf, fFf)
GOW(tanh, dFd)
GOW(tanhf, fFf)
#ifdef HAVE_LD80BITS
GOW(tanhl, DFD)
GOW(tanl, DFD)
#else
GO2(tanhl, KFK, tanh)
GO2(tanl, KFK, tan)
#endif
GOW(tgamma, dFd)
GOW(tgammaf, fFf)
#ifdef HAVE_LD80BITS
GOW(tgammal, DFD)
#else
GO2(tgammal, KFK, tgamma)
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
#ifdef HAVE_LD80BITS
GO(ynl, DFiD)
#else
GO2(ynl, KFiK, yn)
#endif
