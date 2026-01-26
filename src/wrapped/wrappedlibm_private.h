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
GOWD(acoshl, DFD, acosh)
GOWD(acosl, DFD, acos)
GOW(asin, dFd)
GOW(asinf, fFf)
GOM(__asinf_finite, fFf)
GOM(__asin_finite, dFd)
GOW(asinh, dFd)
GOW(asinhf, fFf)
GOWD(asinhl, DFD, asinh)
GOWD(asinl, DFD, asin)
GO(atan, dFd)
GOW(atan2, dFdd)
GOW(atan2f, fFff)
GOM(__atan2f_finite, fFff)
GOM(__atan2_finite, dFdd)
GOWD(atan2l, lFDD, atan2)
GOW(atanf, fFf)
GOW(atanh, dFd)
GOW(atanhf, fFf)
// __atanhf_finite
// __atanh_finite
GOWD(atanhl, DFD, atanh)
GOWD(atanl, DFD, atan)
GOW(cabs, XFX)
GOW(cabsf, xFx)
GOWD(cabsl, DFY, cabs)
GOW(cacos, XFX)
GOW(cacosf, xFx)
GOW(cacosh, XFX)
GOW(cacoshf, xFx)
GOWD(cacoshl, YFY, cacosh)
GOWD(cacosl, YFY, cacos)
GOW(carg, XFX)
GOW(cargf, xFx)
// cargl    // Weak
GOW(casin, XFX)
GOW(casinf, xFx)
GOW(casinh, XFX)
GOW(casinhf, xFx)
GOWD(casinhl, YFY, casinh)
GOWD(casinl, YFY, casin)
GOW(catan, XFX)
GOW(catanf, xFx)
GOW(catanh, XFX)
GOW(catanhf, xFx)
GOWD(catanhl, YFY, catanh)
GOWD(catanl, YFY, catan)
GOW(cbrt, dFd)
GOW(cbrtf, fFf)
GOWD(cbrtl, DFD, cbrt)
GOW(ccos, XFX)
GOW(ccosf, xFx)
GOW(ccosh, XFX)
GOW(ccoshf, xFx)
GOWD(ccoshl, YFY, ccosh)
GOWD(ccosl, YFY, ccos)
GOW(ceil, dFd)
GOW(ceilf, fFf)
GOWD(ceill, DFD, ceil)
GOW(cexp, XFX)
GOW(cexpf, xFx)
GOWD(cexpl, YFY, cexp)
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
GOWD(clogl, YFY, clog)
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
GOWD(coshl, DFD, cosh)
GOWD(cosl, DFD, cos)
GOW(cpow, XFXX)
GOW(cpowf, xFxx)
GOWD(cpowl, YFYY, cpow)
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
GOWD(csinhl, YFY, csinh)
GOWD(csinl, YFY, csin)
GOW(csqrt, XFX)
GOW(csqrtf, xFx)
GOWD(csqrtl, YFY, csqrt)
GOW(ctan, XFX)
GOW(ctanf, xFx)
GOW(ctanh, XFX)
GOW(ctanhf, xFx)
GOWD(ctanhl, YFY, ctanh)
GOWD(ctanl, YFY, ctan)
// __cxa_finalize  // Weak
// drem // Weak
// dremf    // Weak
// dreml    // Weak
GOW(erf, dFd)
GOW(erfc, dFd)
GOW(erfcf, fFf)
GOWD(erfcl, DFD, erfc)
GOW(erff, fFf)
GOWD(erfl, DFD, erf)
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
GOWD(exp2l, DFD, exp2)
GOW(expf, fFf)
GOM(__expf_finite, fFf)
GOM(__exp_finite, dFd)
GOWD(expl, DFD, exp)
GOW(expm1, dFd)
GOW(expm1f, fFf)
GOWD(expm1l, DFD, expm1)
GOW(fabs, dFd)
GOW(fabsf, fFf)
GOWD(fabsl, DFD, fabs)
GOW(fdim, dFdd)
GOW(fdimf, fFff)
// fdiml    // Weak
GOM(feclearexcept, iFEi)
GO(fedisableexcept, iFi)
GO(feenableexcept, iFi)
GO(fegetenv, iFp)
GO(fegetexcept, iFv)
GO(fegetexceptflag, iFpi)
GOM(fegetround, iFEv)
GO(feholdexcept, iFp)
GOM(feraiseexcept, iFEi)
GO(fesetenv, iFp)
GO(fesetexceptflag, iFpi)
GOM(fesetround, iFEi)
GOM(fetestexcept, iFEi)
GO(feupdateenv, iFp)
GOW(finite, iFd)
// __finite
GOW(finitef, iFf)
GO(__finitef, iFf)
// finitel  // Weak
// __finitel
GOW(floor, dFd)
GOW(floorf, fFf)
GOWD(floorl, DFD, floor)
GOW(fma, dFddd)
GOW(fmaf, fFfff)
GOWD(fmal, DFDDD, fma)
GOW(fmax, dFdd)
GOW(fmaxf, fFff)
GOWD(fmaxl, DFDD, fmax)
GOW(fmin, dFdd)
GOW(fminf, fFff)
GOWD(fminl, DFDD, fmin)
GOW(fmod, dFdd)
GOW(fmodf, fFff)
GOM(__fmodf_finite, fFff)
GOM(__fmod_finite, dFdd)
GOWD(fmodl, DFDD, fmod)
GO(__fpclassify, iFd)
GO(__fpclassifyf, iFf)
GOW(frexp, dFdp)
GOW(frexpf, fFfp)
GOWD(frexpl, DFDp, frexp)
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
GOWD(hypotl, DFDD, hypot)
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
GOD(jnl, DFiD, jn)
GOW(ldexp, dFdi)
GOW(ldexpf, fFfi)
GOWD(ldexpl, DFD, ldexp)
GOW(lgamma, dFd)
GOW(lgammaf, fFf)
GOW(lgammaf_r, fFfp)
// __lgammaf_r_finite
GOWD(lgammal, DFD, lgamma)
GOWD(lgammal_r, DFDp, lgamma_r)
GOW(lgamma_r, dFdp)
// __lgamma_r_finite
#ifdef STATICBUILD
//DATAV(_LIB_VERSION, 8)
#else
DATAV(_LIB_VERSION, 8)
#endif
GOWM(llrint, IFEd)
GOWM(llrintf, IFEf)
GOWM(llrintl, IFED)
GOW(llround, IFd)
GOW(llroundf, IFf)
GOWD(llroundl, IFD, llround)
GOW(log, dFd)
GOW(log10, dFd)
GOW(log10f, fFf)
GOM(__log10f_finite, fFf)
GOM(__log10_finite, dFd)
GOWD(log10l, DFD, log10) //Weak
GOW(log1p, dFd)
GOW(log1pf, fFf)
GOWD(log1pl, DFD, log1p)
GOW(log2, dFd)
GOW(log2f, fFf)
GOM(__log2f_finite, fFf)
GOM(__log2_finite, dFd)
GOWD(log2l, DFD, log2)
GOW(logb, dFd)
GOW(logbf, fFf)
// logbl    // Weak
GOW(logf, fFf)
GOM(__logf_finite, fFf)
GOM(__log_finite, dFd)
GOWD(logl, DFD, log)
GOWM(lrint, iFEd)
GOWM(lrintf, iFEf)
// lrintl   // Weak
GOW(lround, iFd)
GOW(lroundf, iFf)
GOWD(lroundl, lFD, lround)
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
GOWD(nextafterl, DFDD, nextafter)
GOWD(nexttoward, dFdD, my_nexttoward) // This loses precision
GOWD(nexttowardf, fFfD, my_nexttowardf) // This loses precision
// nexttowardl  // Weak
GOW(pow, dFdd)
GOWM(pow10, dFd)
GOWM(pow10f, fFf)
GOWM(pow10l, DFD)
GOW(powf, fFff)
GOM(__powf_finite, fFff)
GOM(__pow_finite, dFdd)
GOM(__powl_finite, DFDD)
GOWD(powl, DFDD, pow)
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
GOWD(rintl, DFD, rint)
GOW(round, dFd)
GOW(roundf, fFf)
GO(roundeven, dFd)  //since C23
GO(roundevenf, fFf)
//GO(roundevenl, DFD)
GOWD(roundl, DFD, round)
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
GOWD(sincosl, vFDpp, sincos)
GOW(sinf, fFf)
GOW(sinh, dFd)
GOW(sinhf, fFf)
GOM(__sinhf_finite, fFf)
GOM(__sinh_finite, dFd)
GOWD(sinhl, DFD, sinh)
GOWD(sinl, DFD, sin)
GOW(sqrt, dFd)
GOW(sqrtf, fFf)
GOM(__sqrtf_finite, fFf)
GOM(__sqrt_finite, dFd)
GOWD(sqrtl, DFD, sqrt)
GO(tan, dFd)
GOW(tanf, fFf)
GOW(tanh, dFd)
GOW(tanhf, fFf)
GOWD(tanhl, DFD, tanh)
GOWD(tanl, DFD, tan)
GOW(tgamma, dFd)
GOW(tgammaf, fFf)
GOWD(tgammal, DFD, tgamma)
GOW(trunc, dFd)
GOW(truncf, fFf)
GOWD(truncl, DFD, trunc)
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
GOD(ynl, DFiD, yn)
