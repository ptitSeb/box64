#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error Meh...
#endif

GO(cbbcsd_, vFpppppppppppppppppppppppppppppLLLLL)
GO(cbdsqr_, vFpppppppppppppppL)
GO(cgbbrd_, vFpppppppppppppppppppL)
GO(cgbcon_, vFppppppppppppL)
GO(cgbequ_, vFpppppppppppp)
GO(cgbequb_, vFpppppppppppp)
GO(cgbrfs_, vFpppppppppppppppppppL)
GO(cgbsv_, vFpppppppppp)
GO(cgbsvx_, vFppppppppppppppppppppppppLLL)
//GO(cgbtf2_, 
GO(cgbtrf_, vFpppppppp)
GO(cgbtrs_, vFpppppppppppL)
GO(cgebak_, vFppppppppppLL)
GO(cgebal_, vFppppppppL)
//GO(cgebd2_, 
GO(cgebrd_, vFppppppppppp)
GO(cgecon_, vFpppppppppL)
GO(cgedmd_, vFpppppppppppppppppppppppppppppppLLLL)
GO(cgedmdq_, vFpppppppppppppppppppppppppppppppppppLLLLLL)
GO(cgeequ_, vFpppppppppp)
GO(cgeequb_, vFpppppppppp)
//GOM(cgees_, vFEpppppppppppppppLL)
//GOM(cgeesx_, vFEppppppppppppppppppLLL)
GO(cgeev_, vFppppppppppppppLL)
GO(cgeevx_, vFppppppppppppppppppppppLLLL)
//GO(cgegs_, 
//GO(cgegv_, 
//GO(cgehd2_, 
GO(cgehrd_, vFppppppppp)
GO(cgejsv_, vFpppppppppppppppppppppLLLLLL)
GO(cgelq_, vFppppppppp)
GO(cgelq2_, vFppppppp)
GO(cgelqf_, vFpppppppp)
//GO(cgelqs_, 
//GO(cgelqt_, 
//GO(cgelqt3_, 
GO(cgels_, vFpppppppppppL)
GO(cgelsd_, vFppppppppppppppp)
GO(cgelss_, vFpppppppppppppp)
//GO(cgelst_, 
//GO(cgelsx_, 
GO(cgelsy_, vFpppppppppppppp)
GO(cgemlq_, vFppppppppppppppLL)
//GO(cgemlqt_, 
GO(cgemqr_, vFppppppppppppppLL)
GO(cgemqrt_, vFppppppppppppppLL)
GO(cgeql2_, vFppppppp)
GO(cgeqlf_, vFpppppppp)
GO(cgeqp3_, vFpppppppppp)
//GO(cgeqp3rk_, 
GO(cgeqpf_, vFppppppppp)
GO(cgeqr_, vFppppppppp)
GO(cgeqr2_, vFppppppp)
//GO(cgeqr2p_, 
GO(cgeqrf_, vFpppppppp)
GO(cgeqrfp_, vFpppppppp)
//GO(cgeqrs_, 
GO(cgeqrt_, vFppppppppp)
GO(cgeqrt2_, vFppppppp)
GO(cgeqrt3_, vFppppppp)
GO(cgerfs_, vFpppppppppppppppppL)
GO(cgerq2_, vFppppppp)
GO(cgerqf_, vFpppppppp)
//GO(cgesc2_, 
GO(cgesdd_, vFpppppppppppppppL)
GO(cgesv_, vFpppppppp)
GO(cgesvd_, vFpppppppppppppppLL)
GO(cgesvdq_, vFppppppppppppppppppppppLLLLL)
GO(cgesvdx_, vFppppppppppppppppppppppLLL)
GO(cgesvj_, vFppppppppppppppppLLL)
GO(cgesvx_, vFppppppppppppppppppppppLLL)
//GO(cgetc2_, 
GO(cgetf2_, vFpppppp)
GO(cgetrf_, vFpppppp)
GO(cgetrf2_, vFpppppp)
GO(cgetri_, vFppppppp)
GO(cgetrs_, vFpppppppppL)
GO(cgetsls_, vFpppppppppppL)
GO(cgetsqrhrt_, vFpppppppppppp)
GO(cggbak_, vFpppppppppppLL)
GO(cggbal_, vFppppppppppppL)
//GOM(cgges_, vFEpppppppppppppppppppppLLL)
//GOM(cgges3_, vFEpppppppppppppppppppppLLL)
//GOM(cggesx_, vFEppppppppppppppppppppppppppLLLL)
GO(cggev_, vFpppppppppppppppppLL)
GO(cggev3_, vFpppppppppppppppppLL)
GO(cggevx_, vFpppppppppppppppppppppppppppppLLLL)
GO(cggglm_, vFppppppppppppp)
GO(cgghd3_, vFppppppppppppppppLL)
GO(cgghrd_, vFppppppppppppppLL)
GO(cgglse_, vFppppppppppppp)
GO(cggqrf_, vFpppppppppppp)
GO(cggrqf_, vFpppppppppppp)
GO(cggsvd_, iFppppppppppppppppppppppppLLL)
GO(cggsvd3_, vFpppppppppppppppppppppppppLLL)
GO(cggsvp_, iFpppppppppppppppppppppppppLLL)
GO(cggsvp3_, vFppppppppppppppppppppppppppLLL)
//GO(cgsvj0_, 
//GO(cgsvj1_, 
GO(cgtcon_, vFpppppppppppL)
GO(cgtrfs_, vFppppppppppppppppppppL)
GO(cgtsv_, vFpppppppp)
GO(cgtsvx_, vFppppppppppppppppppppppLL)
GO(cgttrf_, vFppppppp)
GO(cgttrs_, vFpppppppppppL)
//GO(cgtts2_, 
//GO(chb2st_kernels_, 
GO(chbev_, vFppppppppppppLL)
GO(chbev_2stage_, vFpppppppppppppLL)
GO(chbevd_, vFppppppppppppppppLL)
GO(chbevd_2stage_, vFppppppppppppppppLL)
GO(chbevx_, vFpppppppppppppppppppppppLLL)
GO(chbevx_2stage_, vFppppppppppppppppppppppppLLL)
GO(chbgst_, vFppppppppppppppLL)
GO(chbgv_, vFpppppppppppppppLL)
GO(chbgvd_, vFpppppppppppppppppppLL)
GO(chbgvx_, vFppppppppppppppppppppppppppLLL)
GO(chbtrd_, vFppppppppppppLL)
GO(checon_, vFpppppppppL)
GO(checon_3_, vFppppppppppL)
//GO(checon_rook_, 
GO(cheequb_, vFpppppppppL)
GO(cheev_, vFppppppppppLL)
GO(cheev_2stage_, vFppppppppppLL)
GO(cheevd_, vFpppppppppppppLL)
GO(cheevd_2stage_, vFpppppppppppppLL)
GO(cheevr_, vFpppppppppppppppppppppppLLL)
GO(cheevr_2stage_, vFpppppppppppppppppppppppLLL)
GO(cheevx_, vFpppppppppppppppppppppLLL)
GO(cheevx_2stage_, vFpppppppppppppppppppppLLL)
//GO(chegs2_, 
GO(chegst_, vFppppppppL)
GO(chegv_, vFpppppppppppppLL)
GO(chegv_2stage_, vFpppppppppppppLL)
GO(chegvd_, vFppppppppppppppppLL)
GO(chegvx_, vFppppppppppppppppppppppppLLL)
GO(cherfs_, vFpppppppppppppppppL)
GO(chesv_, vFpppppppppppL)
GO(chesv_aa_, vFpppppppppppL)
GO(chesv_aa_2stage_, vFppppppppppppppL)
GO(chesv_rk_, vFppppppppppppL)
GO(chesv_rook_, vFpppppppppppL)
GO(chesvx_, vFppppppppppppppppppppLL)
GO(cheswapr_, vFppppppL)
//GO(chetd2_, 
//GO(chetf2_, 
//GO(chetf2_rk_, 
//GO(chetf2_rook_, 
GO(chetrd_, vFppppppppppL)
GO(chetrd_2stage_, vFpppppppppppppLL)
//GO(chetrd_hb2st_, 
//GO(chetrd_he2hb_, 
GO(chetrf_, vFppppppppL)
GO(chetrf_aa_, vFppppppppL)
GO(chetrf_aa_2stage_, vFpppppppppppL)
GO(chetrf_rk_, vFpppppppppL)
GO(chetrf_rook_, vFppppppppL)
GO(chetri_, vFpppppppL)
GO(chetri2_, vFppppppppL)
GO(chetri2x_, vFppppppppL)
GO(chetri_3_, vFpppppppppL)
//GO(chetri_3x_, 
//GO(chetri_rook_, 
GO(chetrs_, vFpppppppppL)
GO(chetrs2_, vFppppppppppL)
GO(chetrs_3_, vFppppppppppL)
GO(chetrs_aa_, vFpppppppppppL)
GO(chetrs_aa_2stage_, vFppppppppppppL)
GO(chetrs_rook_, vFpppppppppL)
GO(chfrk_, vFppppppppppLLL)
GO(chgeqz_, vFppppppppppppppppppppLLL)
//GO(chla_transtype_, 
GO(chpcon_, vFppppppppL)
GO(chpev_, vFppppppppppLL)
GO(chpevd_, vFppppppppppppppLL)
GO(chpevx_, vFpppppppppppppppppppLLL)
GO(chpgst_, vFppppppL)
GO(chpgv_, vFppppppppppppLL)
GO(chpgvd_, vFppppppppppppppppLL)
GO(chpgvx_, vFpppppppppppppppppppppLLL)
GO(chprfs_, vFpppppppppppppppL)
GO(chpsv_, vFppppppppL)
GO(chpsvx_, vFpppppppppppppppppLL)
GO(chptrd_, vFpppppppL)
GO(chptrf_, vFpppppL)
GO(chptri_, vFppppppL)
GO(chptrs_, vFppppppppL)
GO(chsein_, vFpppppppppppppppppppLLL)
GO(chseqr_, vFpppppppppppppLL)
//GO(clabrd_, 
GO(clacgv_, vFppp)
GO(clacn2_, vFpppppp)
//GO(clacon_, 
GO(clacp2_, vFpppppppL)
GO(clacpy_, vFpppppppL)
GO(clacrm_, vFppppppppp)
//GO(clacrt_, 
//GO(cladiv_, 
//GO(claed0_, 
//GO(claed7_, 
//GO(claed8_, 
//GO(claein_, 
//GO(claesy_, 
//GO(claev2_, 
GO(clag2z_, vFppppppp)
//GO(clags2_, 
//GO(clagtm_, 
//GO(clahef_, 
//GO(clahef_aa_, 
//GO(clahef_rk_, 
//GO(clahef_rook_, 
//GO(clahqr_, 
//GO(clahr2_, 
//GO(clahrd_, 
//GO(claic1_, 
//GO(clals0_, 
//GO(clalsa_, 
//GO(clalsd_, 
//GO(clamswlq_, 
//GO(clamtsqr_, 
GO(clangb_, fFpppppppL)
GO(clange_, fFppppppL)
GO(clangt_, fFpppppL)
GO(clanhb_, fFpppppppLL)
GO(clanhe_, fFppppppLL)
//GO(clanhf_, 
GO(clanhp_, fFpppppLL)
GO(clanhs_, fFpppppL)
GO(clanht_, fFppppL)
GO(clansb_, fFpppppppLL)
GO(clansp_, fFpppppLL)
GO(clansy_, fFppppppLL)
GO(clantb_, fFppppppppLLL)
GO(clantp_, fFppppppLLL)
GO(clantr_, fFppppppppLLL)
//GO(clapll_, 
GO(clapmr_, vFpppppp)
GO(clapmt_, vFpppppp)
//GO(claqgb_, 
//GO(claqge_, 
//GO(claqhb_, 
//GO(claqhe_, 
//GO(claqhp_, 
//GO(claqp2_, 
//GO(claqp2rk_, 
//GO(claqp3rk_, 
//GO(claqps_, 
//GO(claqr0_, 
//GO(claqr1_, 
//GO(claqr2_, 
//GO(claqr3_, 
//GO(claqr4_, 
//GO(claqr5_, 
//GO(claqsb_, 
//GO(claqsp_, 
//GO(claqsy_, 
//GO(claqz0_, 
//GO(claqz1_, 
//GO(claqz2_, 
//GO(claqz3_, 
//GO(clar1v_, 
//GO(clar2v_, 
GO(clarcm_, vFppppppppp)
GO(clarf_, vFpppppppppL)
//GO(clarf1f_, 
//GO(clarf1l_, 
GO(clarfb_, vFpppppppppppppppLLLL)
//GO(clarfb_gett_, 
GO(clarfg_, vFppppp)
//GO(clarfgp_, 
GO(clarft_, vFpppppppppLL)
GO(clarfx_, vFppppppppL)
//GO(clarfy_, 
//GO(clargv_, 
GO(clarnv_, vFpppp)
//GO(clarrv_, 
//GO(clartg_, 
//GO(clartv_, 
//GO(clarz_, 
//GO(clarzb_, 
//GO(clarzt_, 
GO(clascl_, vFppppppppppL)
GO(claset_, vFpppppppL)
//GO(clasr_, 
GO(classq_, vFppppp)
//GO(claswlq_, 
GO(claswp_, vFppppppp)
//GO(clasyf_, 
//GO(clasyf_aa_, 
//GO(clasyf_rk_, 
//GO(clasyf_rook_, 
//GO(clatbs_, 
//GO(clatdf_, 
//GO(clatps_, 
//GO(clatrd_, 
//GO(clatrs_, 
//GO(clatrs3_, 
//GO(clatrz_, 
//GO(clatsqr_, 
//GO(clatzm_, 
//GO(claunhr_col_getrfnp_, 
//GO(claunhr_col_getrfnp2_, 
//GO(clauu2_, 
GO(clauum_, vFpppppL)
GO(cpbcon_, vFppppppppppL)
GO(cpbequ_, vFpppppppppL)
GO(cpbrfs_, vFpppppppppppppppppL)
GO(cpbstf_, vFppppppL)
GO(cpbsv_, vFpppppppppL)
GO(cpbsvx_, vFpppppppppppppppppppppLLL)
//GO(cpbtf2_, 
GO(cpbtrf_, vFppppppL)
GO(cpbtrs_, vFpppppppppL)
GO(cpftrf_, vFpppppLL)
GO(cpftri_, vFpppppLL)
GO(cpftrs_, vFppppppppLL)
GO(cpocon_, vFpppppppppL)
GO(cpoequ_, vFppppppp)
GO(cpoequb_, vFppppppp)
GO(cporfs_, vFppppppppppppppppL)
GO(cposv_, vFppppppppL)
GO(cposvx_, vFppppppppppppppppppppLLL)
GO(cpotf2_, vFpppppL)
GO(cpotrf_, vFpppppL)
GO(cpotrf2_, vFpppppL)
GO(cpotri_, vFpppppL)
GO(cpotrs_, vFppppppppL)
GO(cppcon_, vFppppppppL)
GO(cppequ_, vFpppppppL)
GO(cpprfs_, vFppppppppppppppL)
GO(cppsv_, vFpppppppL)
GO(cppsvx_, vFppppppppppppppppppLLL)
GO(cpptrf_, vFppppL)
GO(cpptri_, vFppppL)
GO(cpptrs_, vFpppppppL)
//GO(cpstf2_, 
GO(cpstrf_, vFpppppppppL)
GO(cptcon_, vFppppppp)
GO(cpteqr_, vFppppppppL)
GO(cptrfs_, vFppppppppppppppppL)
GO(cptsv_, vFppppppp)
GO(cptsvx_, vFpppppppppppppppppL)
GO(cpttrf_, vFpppp)
GO(cpttrs_, vFppppppppL)
//GO(cptts2_, 
//GO(crot_, 
//GO(crscl_, 
GO(cspcon_, vFppppppppL)
//GO(cspmv_, 
//GO(cspr_, 
GO(csprfs_, vFpppppppppppppppL)
GO(cspsv_, vFppppppppL)
GO(cspsvx_, vFpppppppppppppppppLL)
GO(csptrf_, vFpppppL)
GO(csptri_, vFppppppL)
GO(csptrs_, vFppppppppL)
//GO(csrscl_, 
GO(cstedc_, vFpppppppppppppL)
GO(cstegr_, vFppppppppppppppppppppLL)
GO(cstein_, vFppppppppppppp)
GO(cstemr_, vFpppppppppppppppppppppLL)
GO(csteqr_, vFppppppppL)
GO(csycon_, vFpppppppppL)
GO(csycon_3_, vFppppppppppL)
//GO(csycon_rook_, 
GO(csyconv_, vFppppppppLL)
//GO(csyconvf_, 
//GO(csyconvf_rook_, 
GO(csyequb_, vFpppppppppL)
//GO(csymv_, 
GO(csyr_, vFpppppppL)
GO(csyrfs_, vFpppppppppppppppppL)
GO(csysv_, vFpppppppppppL)
GO(csysv_aa_, vFpppppppppppL)
GO(csysv_aa_2stage_, vFppppppppppppppL)
GO(csysv_rk_, vFppppppppppppL)
GO(csysv_rook_, vFpppppppppppL)
GO(csysvx_, vFppppppppppppppppppppLL)
GO(csyswapr_, vFppppppL)
//GO(csytf2_, 
//GO(csytf2_rk_, 
//GO(csytf2_rook_, 
GO(csytrf_, vFppppppppL)
GO(csytrf_aa_, vFppppppppL)
GO(csytrf_aa_2stage_, vFpppppppppppL)
GO(csytrf_rk_, vFpppppppppL)
GO(csytrf_rook_, vFppppppppL)
GO(csytri_, vFpppppppL)
GO(csytri2_, vFppppppppL)
GO(csytri2x_, vFppppppppL)
GO(csytri_3_, vFpppppppppL)
//GO(csytri_3x_, 
//GO(csytri_rook_, 
GO(csytrs_, vFpppppppppL)
GO(csytrs2_, vFppppppppppL)
GO(csytrs_3_, vFppppppppppL)
GO(csytrs_aa_, vFpppppppppppL)
GO(csytrs_aa_2stage_, vFppppppppppppL)
GO(csytrs_rook_, vFpppppppppL)
GO(ctbcon_, vFpppppppppppLLL)
GO(ctbrfs_, vFpppppppppppppppppLLL)
GO(ctbtrs_, vFpppppppppppLLL)
GO(ctfsm_, vFpppppppppppLLLLL)
GO(ctftri_, vFppppppLLL)
GO(ctfttp_, vFppppppLL)
GO(ctfttr_, vFpppppppLL)
GO(ctgevc_, vFpppppppppppppppppLL)
//GO(ctgex2_, 
GO(ctgexc_, vFpppppppppppppp)
GO(ctgsen_, vFpppppppppppppppppppppppp)
GO(ctgsja_, vFpppppppppppppppppppppppppLLL)
GO(ctgsna_, vFppppppppppppppppppppLL)
//GO(ctgsy2_, 
GO(ctgsyl_, vFppppppppppppppppppppppL)
GO(ctpcon_, vFpppppppppLLL)
GO(ctplqt_, vFpppppppppppp)
GO(ctplqt2_, vFpppppppppp)
GO(ctpmlqt_, vFpppppppppppppppppLL)
GO(ctpmqrt_, vFpppppppppppppppppLL)
GO(ctpqrt_, vFpppppppppppp)
GO(ctpqrt2_, vFpppppppppp)
GO(ctprfb_, vFppppppppppppppppppLLLL)
GO(ctprfs_, vFpppppppppppppppLLL)
GO(ctptri_, vFpppppLL)
GO(ctptrs_, vFpppppppppLLL)
GO(ctpttf_, vFppppppLL)
GO(ctpttr_, vFppppppL)
GO(ctrcon_, vFppppppppppLLL)
GO(ctrevc_, vFpppppppppppppppLL)
GO(ctrevc3_, vFpppppppppppppppppLL)
GO(ctrexc_, vFpppppppppL)
GO(ctrrfs_, vFppppppppppppppppLLL)
GO(ctrsen_, vFpppppppppppppppLL)
GO(ctrsna_, vFppppppppppppppppppLL)
GO(ctrsyl_, vFpppppppppppppLL)
GO(ctrsyl3_, vFpppppppppppppppLL)
//GO(ctrti2_, 
GO(ctrtri_, vFppppppLL)
GO(ctrtrs_, vFppppppppppLLL)
GO(ctrttf_, vFpppppppLL)
GO(ctrttp_, vFppppppL)
//GO(ctzrqf_, 
GO(ctzrzf_, vFpppppppp)
GO(cunbdb_, vFppppppppppppppppppppppLL)
//GO(cunbdb1_, 
//GO(cunbdb2_, 
//GO(cunbdb3_, 
//GO(cunbdb4_, 
//GO(cunbdb5_, 
//GO(cunbdb6_, 
GO(cuncsd_, vFppppppppppppppppppppppppppppppppLLLLLL)
GO(cuncsd2by1_, vFpppppppppppppppppppppppLLL)
//GO(cung2l_, 
//GO(cung2r_, 
GO(cungbr_, vFppppppppppL)
GO(cunghr_, vFppppppppp)
//GO(cungl2_, 
GO(cunglq_, vFppppppppp)
GO(cungql_, vFppppppppp)
GO(cungqr_, vFppppppppp)
//GO(cungr2_, 
GO(cungrq_, vFppppppppp)
GO(cungtr_, vFppppppppL)
//GO(cungtsqr_, 
GO(cungtsqr_row_, vFppppppppppp)
GO(cunhr_col_, vFppppppppp)
//GO(cunm22_, 
//GO(cunm2l_, 
//GO(cunm2r_, 
GO(cunmbr_, vFppppppppppppppLLL)
GO(cunmhr_, vFppppppppppppppLL)
//GO(cunml2_, 
GO(cunmlq_, vFpppppppppppppLL)
GO(cunmql_, vFpppppppppppppLL)
GO(cunmqr_, vFpppppppppppppLL)
//GO(cunmr2_, 
//GO(cunmr3_, 
GO(cunmrq_, vFpppppppppppppLL)
GO(cunmrz_, vFppppppppppppppLL)
GO(cunmtr_, vFpppppppppppppLLL)
GO(cupgtr_, vFppppppppL)
GO(cupmtr_, vFpppppppppppLLL)
GO(dbbcsd_, vFpppppppppppppppppppppppppppppLLLLL)
GO(dbdsdc_, vFppppppppppppppLL)
GO(dbdsqr_, vFpppppppppppppppL)
GO(dbdsvdx_, vFpppppppppppppppppLLL)
GO(ddisna_, vFppppppL)
GO(dgbbrd_, vFppppppppppppppppppL)
GO(dgbcon_, vFppppppppppppL)
GO(dgbequ_, vFpppppppppppp)
GO(dgbequb_, vFpppppppppppp)
GO(dgbrfs_, vFpppppppppppppppppppL)
GO(dgbsv_, vFpppppppppp)
GO(dgbsvx_, vFppppppppppppppppppppppppLLL)
//GO(dgbtf2_, 
GO(dgbtrf_, vFpppppppp)
GO(dgbtrs_, vFpppppppppppL)
GO(dgebak_, vFppppppppppLL)
GO(dgebal_, vFppppppppL)
//GO(dgebd2_, 
GO(dgebrd_, vFppppppppppp)
GO(dgecon_, vFpppppppppL)
GO(dgedmd_, vFppppppppppppppppppppppppppppppLLLL)
GO(dgedmdq_, vFppppppppppppppppppppppppppppppppppLLLLLL)
GO(dgeequ_, vFpppppppppp)
GO(dgeequb_, vFpppppppppp)
//GOM(dgees_, vFEpppppppppppppppLL)
//GOM(dgeesx_, vFEppppppppppppppppppppLLL)
GO(dgeev_, vFppppppppppppppLL)
GO(dgeevx_, vFpppppppppppppppppppppppLLLL)
//GO(dgegs_, 
//GO(dgegv_, 
//GO(dgehd2_, 
GO(dgehrd_, vFppppppppp)
GO(dgejsv_, vFpppppppppppppppppppLLLLLL)
GO(dgelq_, vFppppppppp)
GO(dgelq2_, vFppppppp)
GO(dgelqf_, vFpppppppp)
//GO(dgelqs_, 
//GO(dgelqt_, 
//GO(dgelqt3_, 
GO(dgels_, vFpppppppppppL)
GO(dgelsd_, vFpppppppppppppp)
GO(dgelss_, vFppppppppppppp)
//GO(dgelst_, 
//GO(dgelsx_, 
GO(dgelsy_, vFppppppppppppp)
GO(dgemlq_, vFppppppppppppppLL)
//GO(dgemlqt_, 
GO(dgemqr_, vFppppppppppppppLL)
GO(dgemqrt_, vFppppppppppppppLL)
GO(dgeql2_, vFppppppp)
GO(dgeqlf_, vFpppppppp)
GO(dgeqp3_, vFppppppppp)
//GO(dgeqp3rk_, 
GO(dgeqpf_, vFpppppppp)
GO(dgeqr_, vFppppppppp)
GO(dgeqr2_, vFppppppp)
//GO(dgeqr2p_, 
GO(dgeqrf_, vFpppppppp)
GO(dgeqrfp_, vFpppppppp)
//GO(dgeqrs_, 
GO(dgeqrt_, vFppppppppp)
GO(dgeqrt2_, vFppppppp)
GO(dgeqrt3_, vFppppppp)
GO(dgerfs_, vFpppppppppppppppppL)
GO(dgerq2_, vFppppppp)
GO(dgerqf_, vFpppppppp)
//GO(dgesc2_, 
GO(dgesdd_, vFppppppppppppppL)
GO(dgesv_, vFpppppppp)
GO(dgesvd_, vFppppppppppppppLL)
GO(dgesvdq_, vFppppppppppppppppppppppLLLLL)
GO(dgesvdx_, vFpppppppppppppppppppppLLL)
GO(dgesvj_, vFppppppppppppppLLL)
GO(dgesvx_, vFppppppppppppppppppppppLLL)
//GO(dgetc2_, 
GO(dgetf2_, vFpppppp)
GO(dgetrf_, vFpppppp)
GO(dgetrf2_, vFpppppp)
GO(dgetri_, vFppppppp)
GO(dgetrs_, vFpppppppppL)
GO(dgetsls_, vFpppppppppppL)
GO(dgetsqrhrt_, vFpppppppppppp)
GO(dggbak_, vFpppppppppppLL)
GO(dggbal_, vFppppppppppppL)
//GOM(dgges_, vFEpppppppppppppppppppppLLL)
//GOM(dgges3_, vFEpppppppppppppppppppppLLL)
//GOM(dggesx_, vFEppppppppppppppppppppppppppLLLL)
GO(dggev_, vFpppppppppppppppppLL)
GO(dggev3_, vFpppppppppppppppppLL)
GO(dggevx_, vFpppppppppppppppppppppppppppppLLLL)
GO(dggglm_, vFppppppppppppp)
GO(dgghd3_, vFppppppppppppppppLL)
GO(dgghrd_, vFppppppppppppppLL)
GO(dgglse_, vFppppppppppppp)
GO(dggqrf_, vFpppppppppppp)
GO(dggrqf_, vFpppppppppppp)
GO(dggsvd_, iFpppppppppppppppppppppppLLL)
GO(dggsvd3_, vFppppppppppppppppppppppppLLL)
GO(dggsvp_, iFppppppppppppppppppppppppLLL)
GO(dggsvp3_, vFpppppppppppppppppppppppppLLL)
//GO(dgsvj0_, 
//GO(dgsvj1_, 
GO(dgtcon_, vFppppppppppppL)
GO(dgtrfs_, vFppppppppppppppppppppL)
GO(dgtsv_, vFpppppppp)
GO(dgtsvx_, vFppppppppppppppppppppppLL)
GO(dgttrf_, vFppppppp)
GO(dgttrs_, vFpppppppppppL)
//GO(dgtts2_, 
GO(dhgeqz_, vFppppppppppppppppppppLLL)
GO(dhsein_, vFpppppppppppppppppppLLL)
GO(dhseqr_, vFppppppppppppppLL)
//GO(disnan_, 
//GO(dlabad_, 
//GO(dlabrd_, 
GO(dlacn2_, vFppppppp)
//GO(dlacon_, 
GO(dlacpy_, vFpppppppL)
//GO(dladiv_, 
//GO(dladiv1_, 
//GO(dladiv2_, 
//GO(dlae2_, 
//GO(dlaebz_, 
//GO(dlaed0_, 
//GO(dlaed1_, 
//GO(dlaed2_, 
//GO(dlaed3_, 
//GO(dlaed4_, 
//GO(dlaed5_, 
//GO(dlaed6_, 
//GO(dlaed7_, 
//GO(dlaed8_, 
//GO(dlaed9_, 
//GO(dlaeda_, 
//GO(dlaein_, 
//GO(dlaev2_, 
//GO(dlaexc_, 
//GO(dlag2_, 
GO(dlag2s_, vFppppppp)
//GO(dlags2_, 
//GO(dlagtf_, 
//GO(dlagtm_, 
//GO(dlagts_, 
//GO(dlagv2_, 
//GO(dlahqr_, 
//GO(dlahr2_, 
//GO(dlahrd_, 
//GO(dlaic1_, 
//GO(dlaisnan_, 
//GO(dlaln2_, 
//GO(dlals0_, 
//GO(dlalsa_, 
//GO(dlalsd_, 
//GO(dlamc3_, 
GO(dlamch_, dFpL)
//GO(dlamrg_, 
//GO(dlamswlq_, 
//GO(dlamtsqr_, 
//GO(dlaneg_, 
GO(dlangb_, dFpppppppL)
GO(dlange_, dFppppppL)
GO(dlangt_, dFpppppL)
GO(dlanhs_, dFpppppL)
GO(dlansb_, dFpppppppLL)
//GO(dlansf_, 
GO(dlansp_, dFpppppLL)
GO(dlanst_, dFppppL)
GO(dlansy_, dFppppppLL)
GO(dlantb_, dFppppppppLLL)
GO(dlantp_, dFppppppLLL)
GO(dlantr_, dFppppppppLLL)
//GO(dlanv2_, 
//GO(dlaorhr_col_getrfnp_, 
//GO(dlaorhr_col_getrfnp2_, 
//GO(dlapll_, 
GO(dlapmr_, vFpppppp)
GO(dlapmt_, vFpppppp)
GO(dlapy2_, dFpp)
GO(dlapy3_, dFppp)
//GO(dlaqgb_, 
//GO(dlaqge_, 
//GO(dlaqp2_, 
//GO(dlaqp2rk_, 
//GO(dlaqp3rk_, 
//GO(dlaqps_, 
//GO(dlaqr0_, 
//GO(dlaqr1_, 
//GO(dlaqr2_, 
//GO(dlaqr3_, 
//GO(dlaqr4_, 
//GO(dlaqr5_, 
//GO(dlaqsb_, 
//GO(dlaqsp_, 
//GO(dlaqsy_, 
//GO(dlaqtr_, 
//GO(dlaqz0_, 
//GO(dlaqz1_, 
//GO(dlaqz2_, 
//GO(dlaqz3_, 
//GO(dlaqz4_, 
//GO(dlar1v_, 
//GO(dlar2v_, 
GO(dlarf_, vFpppppppppL)
//GO(dlarf1f_, 
//GO(dlarf1l_, 
GO(dlarfb_, vFpppppppppppppppLLLL)
//GO(dlarfb_gett_, 
GO(dlarfg_, vFppppp)
//GO(dlarfgp_, 
GO(dlarft_, vFpppppppppLL)
GO(dlarfx_, vFppppppppL)
//GO(dlarfy_, 
//GO(dlargv_, 
//GO(dlarmm_, 
GO(dlarnv_, vFpppp)
//GO(dlarra_, 
//GO(dlarrb_, 
//GO(dlarrc_, 
//GO(dlarrd_, 
//GO(dlarre_, 
//GO(dlarrf_, 
//GO(dlarrj_, 
//GO(dlarrk_, 
//GO(dlarrr_, 
//GO(dlarrv_, 
//GO(dlartg_, 
GO(dlartgp_, vFppppp)
GO(dlartgs_, vFppppp)
//GO(dlartv_, 
//GO(dlaruv_, 
//GO(dlarz_, 
//GO(dlarzb_, 
//GO(dlarzt_, 
//GO(dlas2_, 
GO(dlascl_, vFppppppppppL)
//GO(dlasd0_, 
//GO(dlasd1_, 
//GO(dlasd2_, 
//GO(dlasd3_, 
//GO(dlasd4_, 
//GO(dlasd5_, 
//GO(dlasd6_, 
//GO(dlasd7_, 
//GO(dlasd8_, 
//GO(dlasda_, 
//GO(dlasdq_, 
//GO(dlasdt_, 
GO(dlaset_, vFpppppppL)
//GO(dlasq1_, 
//GO(dlasq2_, 
//GO(dlasq3_, 
//GO(dlasq4_, 
//GO(dlasq5_, 
//GO(dlasq6_, 
//GO(dlasr_, 
GO(dlasrt_, vFppppL)
GO(dlassq_, vFppppp)
//GO(dlasv2_, 
//GO(dlaswlq_, 
GO(dlaswp_, vFppppppp)
//GO(dlasy2_, 
//GO(dlasyf_, 
//GO(dlasyf_aa_, 
//GO(dlasyf_rk_, 
//GO(dlasyf_rook_, 
//GO(dlat2s_, 
//GO(dlatbs_, 
//GO(dlatdf_, 
//GO(dlatps_, 
//GO(dlatrd_, 
//GO(dlatrs_, 
//GO(dlatrs3_, 
//GO(dlatrz_, 
//GO(dlatsqr_, 
//GO(dlatzm_, 
//GO(dlauu2_, 
GO(dlauum_, vFpppppL)
GO(dopgtr_, vFppppppppL)
GO(dopmtr_, vFpppppppppppLLL)
GO(dorbdb_, vFppppppppppppppppppppppLL)
//GO(dorbdb1_, 
//GO(dorbdb2_, 
//GO(dorbdb3_, 
//GO(dorbdb4_, 
//GO(dorbdb5_, 
//GO(dorbdb6_, 
GO(dorcsd_, vFppppppppppppppppppppppppppppppLLLLLL)
GO(dorcsd2by1_, vFpppppppppppppppppppppLLL)
//GO(dorg2l_, 
//GO(dorg2r_, 
GO(dorgbr_, vFppppppppppL)
GO(dorghr_, vFppppppppp)
//GO(dorgl2_, 
GO(dorglq_, vFppppppppp)
GO(dorgql_, vFppppppppp)
GO(dorgqr_, vFppppppppp)
//GO(dorgr2_, 
GO(dorgrq_, vFppppppppp)
GO(dorgtr_, vFppppppppL)
//GO(dorgtsqr_, 
GO(dorgtsqr_row_, vFppppppppppp)
GO(dorhr_col_, vFppppppppp)
//GO(dorm22_, 
//GO(dorm2l_, 
//GO(dorm2r_, 
GO(dormbr_, vFppppppppppppppLLL)
GO(dormhr_, vFppppppppppppppLL)
//GO(dorml2_, 
GO(dormlq_, vFpppppppppppppLL)
GO(dormql_, vFpppppppppppppLL)
GO(dormqr_, vFpppppppppppppLL)
//GO(dormr2_, 
//GO(dormr3_, 
GO(dormrq_, vFpppppppppppppLL)
GO(dormrz_, vFppppppppppppppLL)
GO(dormtr_, vFpppppppppppppLLL)
GO(dpbcon_, vFppppppppppL)
GO(dpbequ_, vFpppppppppL)
GO(dpbrfs_, vFpppppppppppppppppL)
GO(dpbstf_, vFppppppL)
GO(dpbsv_, vFpppppppppL)
GO(dpbsvx_, vFpppppppppppppppppppppLLL)
//GO(dpbtf2_, 
GO(dpbtrf_, vFppppppL)
GO(dpbtrs_, vFpppppppppL)
GO(dpftrf_, vFpppppLL)
GO(dpftri_, vFpppppLL)
GO(dpftrs_, vFppppppppLL)
GO(dpocon_, vFpppppppppL)
GO(dpoequ_, vFppppppp)
GO(dpoequb_, vFppppppp)
GO(dporfs_, vFppppppppppppppppL)
GO(dposv_, vFppppppppL)
GO(dposvx_, vFppppppppppppppppppppLLL)
GO(dpotf2_, vFpppppL)
GO(dpotrf_, vFpppppL)
GO(dpotrf2_, vFpppppL)
GO(dpotri_, vFpppppL)
GO(dpotrs_, vFppppppppL)
GO(dppcon_, vFppppppppL)
GO(dppequ_, vFpppppppL)
GO(dpprfs_, vFppppppppppppppL)
GO(dppsv_, vFpppppppL)
GO(dppsvx_, vFppppppppppppppppppLLL)
GO(dpptrf_, vFppppL)
GO(dpptri_, vFppppL)
GO(dpptrs_, vFpppppppL)
//GO(dpstf2_, 
GO(dpstrf_, vFpppppppppL)
GO(dptcon_, vFppppppp)
GO(dpteqr_, vFppppppppL)
GO(dptrfs_, vFpppppppppppppp)
GO(dptsv_, vFppppppp)
GO(dptsvx_, vFppppppppppppppppL)
GO(dpttrf_, vFpppp)
GO(dpttrs_, vFppppppp)
//GO(dptts2_, 
//GO(droundup_lwork_, 
//GO(drscl_, 
//GO(dsb2st_kernels_, 
GO(dsbev_, vFpppppppppppLL)
GO(dsbev_2stage_, vFppppppppppppLL)
GO(dsbevd_, vFppppppppppppppLL)
GO(dsbevd_2stage_, vFppppppppppppppLL)
GO(dsbevx_, vFppppppppppppppppppppppLLL)
GO(dsbevx_2stage_, vFpppppppppppppppppppppppLLL)
GO(dsbgst_, vFpppppppppppppLL)
GO(dsbgv_, vFppppppppppppppLL)
GO(dsbgvd_, vFpppppppppppppppppLL)
GO(dsbgvx_, vFpppppppppppppppppppppppppLLL)
GO(dsbtrd_, vFppppppppppppLL)
//GO(dsecnd_, 
GO(dsfrk_, vFppppppppppLLL)
GO(dsgesv_, vFppppppppppppp)
GO(dspcon_, vFpppppppppL)
GO(dspev_, vFpppppppppLL)
GO(dspevd_, vFppppppppppppLL)
GO(dspevx_, vFppppppppppppppppppLLL)
GO(dspgst_, vFppppppL)
GO(dspgv_, vFpppppppppppLL)
GO(dspgvd_, vFppppppppppppppLL)
GO(dspgvx_, vFppppppppppppppppppppLLL)
GO(dsposv_, vFpppppppppppppL)
GO(dsprfs_, vFpppppppppppppppL)
GO(dspsv_, vFppppppppL)
GO(dspsvx_, vFpppppppppppppppppLL)
GO(dsptrd_, vFpppppppL)
GO(dsptrf_, vFpppppL)
GO(dsptri_, vFppppppL)
GO(dsptrs_, vFppppppppL)
GO(dstebz_, vFppppppppppppppppppLL)
GO(dstedc_, vFpppppppppppL)
GO(dstegr_, vFppppppppppppppppppppLL)
GO(dstein_, vFppppppppppppp)
GO(dstemr_, vFpppppppppppppppppppppLL)
GO(dsteqr_, vFppppppppL)
GO(dsterf_, vFpppp)
GO(dstev_, vFppppppppL)
GO(dstevd_, vFpppppppppppL)
GO(dstevr_, vFppppppppppppppppppppLL)
GO(dstevx_, vFppppppppppppppppppLL)
GO(dsycon_, vFppppppppppL)
GO(dsycon_3_, vFpppppppppppL)
//GO(dsycon_rook_, 
GO(dsyconv_, vFppppppppLL)
//GO(dsyconvf_, 
//GO(dsyconvf_rook_, 
GO(dsyequb_, vFpppppppppL)
GO(dsyev_, vFpppppppppLL)
GO(dsyev_2stage_, vFpppppppppLL)
GO(dsyevd_, vFpppppppppppLL)
GO(dsyevd_2stage_, vFpppppppppppLL)
GO(dsyevr_, vFpppppppppppppppppppppLLL)
GO(dsyevr_2stage_, vFpppppppppppppppppppppLLL)
GO(dsyevx_, vFppppppppppppppppppppLLL)
GO(dsyevx_2stage_, vFppppppppppppppppppppLLL)
//GO(dsygs2_, 
GO(dsygst_, vFppppppppL)
GO(dsygv_, vFppppppppppppLL)
GO(dsygv_2stage_, vFppppppppppppLL)
GO(dsygvd_, vFppppppppppppppLL)
GO(dsygvx_, vFpppppppppppppppppppppppLLL)
GO(dsyrfs_, vFpppppppppppppppppL)
GO(dsysv_, vFpppppppppppL)
GO(dsysv_aa_, vFpppppppppppL)
GO(dsysv_aa_2stage_, vFppppppppppppppL)
GO(dsysv_rk_, vFppppppppppppL)
GO(dsysv_rook_, vFpppppppppppL)
GO(dsysvx_, vFppppppppppppppppppppLL)
GO(dsyswapr_, vFppppppL)
//GO(dsytd2_, 
//GO(dsytf2_, 
//GO(dsytf2_rk_, 
//GO(dsytf2_rook_, 
GO(dsytrd_, vFppppppppppL)
GO(dsytrd_2stage_, vFpppppppppppppLL)
//GO(dsytrd_sb2st_, 
//GO(dsytrd_sy2sb_, 
GO(dsytrf_, vFppppppppL)
GO(dsytrf_aa_, vFppppppppL)
GO(dsytrf_aa_2stage_, vFpppppppppppL)
GO(dsytrf_rk_, vFpppppppppL)
GO(dsytrf_rook_, vFppppppppL)
GO(dsytri_, vFpppppppL)
GO(dsytri2_, vFppppppppL)
GO(dsytri2x_, vFppppppppL)
GO(dsytri_3_, vFpppppppppL)
//GO(dsytri_3x_, 
//GO(dsytri_rook_, 
GO(dsytrs_, vFpppppppppL)
GO(dsytrs2_, vFppppppppppL)
GO(dsytrs_3_, vFppppppppppL)
GO(dsytrs_aa_, vFpppppppppppL)
GO(dsytrs_aa_2stage_, vFppppppppppppL)
GO(dsytrs_rook_, vFpppppppppL)
GO(dtbcon_, vFpppppppppppLLL)
GO(dtbrfs_, vFpppppppppppppppppLLL)
GO(dtbtrs_, vFpppppppppppLLL)
GO(dtfsm_, vFpppppppppppLLLLL)
GO(dtftri_, vFppppppLLL)
GO(dtfttp_, vFppppppLL)
GO(dtfttr_, vFpppppppLL)
GO(dtgevc_, vFppppppppppppppppLL)
//GO(dtgex2_, 
GO(dtgexc_, vFpppppppppppppppp)
GO(dtgsen_, vFppppppppppppppppppppppppp)
GO(dtgsja_, vFpppppppppppppppppppppppppLLL)
GO(dtgsna_, vFppppppppppppppppppppLL)
//GO(dtgsy2_, 
GO(dtgsyl_, vFppppppppppppppppppppppL)
GO(dtpcon_, vFpppppppppLLL)
GO(dtplqt_, vFpppppppppppp)
GO(dtplqt2_, vFpppppppppp)
GO(dtpmlqt_, vFpppppppppppppppppLL)
GO(dtpmqrt_, vFpppppppppppppppppLL)
GO(dtpqrt_, vFpppppppppppp)
GO(dtpqrt2_, vFpppppppppp)
GO(dtprfb_, vFppppppppppppppppppLLLL)
GO(dtprfs_, vFpppppppppppppppLLL)
GO(dtptri_, vFpppppLL)
GO(dtptrs_, vFpppppppppLLL)
GO(dtpttf_, vFppppppLL)
GO(dtpttr_, vFppppppL)
GO(dtrcon_, vFppppppppppLLL)
GO(dtrevc_, vFppppppppppppppLL)
GO(dtrevc3_, vFpppppppppppppppLL)
GO(dtrexc_, vFppppppppppL)
GO(dtrrfs_, vFppppppppppppppppLLL)
GO(dtrsen_, vFppppppppppppppppppLL)
GO(dtrsna_, vFppppppppppppppppppLL)
GO(dtrsyl_, vFpppppppppppppLL)
GO(dtrsyl3_, vFpppppppppppppppppLL)
//GO(dtrti2_, 
GO(dtrtri_, vFppppppLL)
GO(dtrtrs_, vFppppppppppLLL)
GO(dtrttf_, vFpppppppLL)
GO(dtrttp_, vFppppppL)
//GO(dtzrqf_, 
GO(dtzrzf_, vFpppppppp)
//GO(dzsum1_, 
//GO(icmax1_, 
//GO(ieeeck_, 
//GO(ilaclc_, 
//GO(ilaclr_, 
//GO(iladiag_, 
//GO(iladlc_, 
//GO(iladlr_, 
//GO(ilaenv_, 
//GO(ilaenv2stage_, 
//GO(ilaprec_, 
//GO(ilaslc_, 
//GO(ilaslr_, 
//GO(ilatrans_, 
//GO(ilauplo_, 
GO(ilaver_, vFppp)
//GO(ilazlc_, 
//GO(ilazlr_, 
//GO(iparam2stage_, 
//GO(iparmq_, 
//GO(izmax1_, 
//GO(__la_xisnan_MOD_disnan, 
//GO(__la_xisnan_MOD_sisnan, 
GO(lsame_, iFppLL)
//GO(lsamen_, 
GO(sbbcsd_, vFpppppppppppppppppppppppppppppLLLLL)
GO(sbdsdc_, vFppppppppppppppLL)
GO(sbdsqr_, vFpppppppppppppppL)
GO(sbdsvdx_, vFpppppppppppppppppLLL)
//GO(scsum1_, 
GO(sdisna_, vFppppppL)
//GO(second_, 
GO(sgbbrd_, vFppppppppppppppppppL)
GO(sgbcon_, vFppppppppppppL)
GO(sgbequ_, vFpppppppppppp)
GO(sgbequb_, vFpppppppppppp)
GO(sgbrfs_, vFpppppppppppppppppppL)
GO(sgbsv_, vFpppppppppp)
GO(sgbsvx_, vFppppppppppppppppppppppppLLL)
//GO(sgbtf2_, 
GO(sgbtrf_, vFpppppppp)
GO(sgbtrs_, vFpppppppppppL)
GO(sgebak_, vFppppppppppLL)
GO(sgebal_, vFppppppppL)
//GO(sgebd2_, 
GO(sgebrd_, vFppppppppppp)
GO(sgecon_, vFpppppppppL)
GO(sgedmd_, vFppppppppppppppppppppppppppppppLLLL)
GO(sgedmdq_, vFppppppppppppppppppppppppppppppppppLLLLLL)
GO(sgeequ_, vFpppppppppp)
GO(sgeequb_, vFpppppppppp)
//GOM(sgees_, vFEpppppppppppppppLL)
//GOM(sgeesx_, vFEppppppppppppppppppppLLL)
GO(sgeev_, vFppppppppppppppLL)
GO(sgeevx_, vFpppppppppppppppppppppppLLLL)
//GO(sgegs_, 
//GO(sgegv_, 
//GO(sgehd2_, 
GO(sgehrd_, vFppppppppp)
GO(sgejsv_, vFpppppppppppppppppppLLLLLL)
GO(sgelq_, vFppppppppp)
GO(sgelq2_, vFppppppp)
GO(sgelqf_, vFpppppppp)
//GO(sgelqs_, 
//GO(sgelqt_, 
//GO(sgelqt3_, 
GO(sgels_, vFpppppppppppL)
GO(sgelsd_, vFpppppppppppppp)
GO(sgelss_, vFppppppppppppp)
//GO(sgelst_, 
//GO(sgelsx_, 
GO(sgelsy_, vFppppppppppppp)
GO(sgemlq_, vFppppppppppppppLL)
//GO(sgemlqt_, 
GO(sgemqr_, vFppppppppppppppLL)
GO(sgemqrt_, vFppppppppppppppLL)
GO(sgeql2_, vFppppppp)
GO(sgeqlf_, vFpppppppp)
GO(sgeqp3_, vFppppppppp)
//GO(sgeqp3rk_, 
GO(sgeqpf_, vFpppppppp)
GO(sgeqr_, vFppppppppp)
GO(sgeqr2_, vFppppppp)
//GO(sgeqr2p_, 
GO(sgeqrf_, vFpppppppp)
GO(sgeqrfp_, vFpppppppp)
//GO(sgeqrs_, 
GO(sgeqrt_, vFppppppppp)
GO(sgeqrt2_, vFppppppp)
GO(sgeqrt3_, vFppppppp)
GO(sgerfs_, vFpppppppppppppppppL)
GO(sgerq2_, vFppppppp)
GO(sgerqf_, vFpppppppp)
//GO(sgesc2_, 
GO(sgesdd_, vFppppppppppppppL)
GO(sgesv_, vFpppppppp)
GO(sgesvd_, vFppppppppppppppLL)
GO(sgesvdq_, vFppppppppppppppppppppppLLLLL)
GO(sgesvdx_, vFpppppppppppppppppppppLLL)
GO(sgesvj_, vFppppppppppppppLLL)
GO(sgesvx_, vFppppppppppppppppppppppLLL)
//GO(sgetc2_, 
GO(sgetf2_, vFpppppp)
GO(sgetrf_, vFpppppp)
GO(sgetrf2_, vFpppppp)
GO(sgetri_, vFppppppp)
GO(sgetrs_, vFpppppppppL)
GO(sgetsls_, vFpppppppppppL)
GO(sgetsqrhrt_, vFpppppppppppp)
GO(sggbak_, vFpppppppppppLL)
GO(sggbal_, vFppppppppppppL)
//GOM(sgges_, vFEpppppppppppppppppppppLLL)
//GOM(sgges3_, vFEpppppppppppppppppppppLLL)
//GOM(sggesx_, vFEppppppppppppppppppppppppppLLLL)
GO(sggev_, vFpppppppppppppppppLL)
GO(sggev3_, vFpppppppppppppppppLL)
GO(sggevx_, vFpppppppppppppppppppppppppppppLLLL)
GO(sggglm_, vFppppppppppppp)
GO(sgghd3_, vFppppppppppppppppLL)
GO(sgghrd_, vFppppppppppppppLL)
GO(sgglse_, vFppppppppppppp)
GO(sggqrf_, vFpppppppppppp)
GO(sggrqf_, vFpppppppppppp)
GO(sggsvd_, iFpppppppppppppppppppppppLLL)
GO(sggsvd3_, vFppppppppppppppppppppppppLLL)
GO(sggsvp_, iFppppppppppppppppppppppppLLL)
GO(sggsvp3_, vFpppppppppppppppppppppppppLLL)
//GO(sgsvj0_, 
//GO(sgsvj1_, 
GO(sgtcon_, vFppppppppppppL)
GO(sgtrfs_, vFppppppppppppppppppppL)
GO(sgtsv_, vFpppppppp)
GO(sgtsvx_, vFppppppppppppppppppppppLL)
GO(sgttrf_, vFppppppp)
GO(sgttrs_, vFpppppppppppL)
//GO(sgtts2_, 
GO(shgeqz_, vFppppppppppppppppppppLLL)
GO(shsein_, vFpppppppppppppppppppLLL)
GO(shseqr_, vFppppppppppppppLL)
//GO(sisnan_, 
//GO(slabad_, 
//GO(slabrd_, 
GO(slacn2_, vFppppppp)
//GO(slacon_, 
GO(slacpy_, vFpppppppL)
//GO(sladiv_, 
//GO(sladiv1_, 
//GO(sladiv2_, 
//GO(slae2_, 
//GO(slaebz_, 
//GO(slaed0_, 
//GO(slaed1_, 
//GO(slaed2_, 
//GO(slaed3_, 
//GO(slaed4_, 
//GO(slaed5_, 
//GO(slaed6_, 
//GO(slaed7_, 
//GO(slaed8_, 
//GO(slaed9_, 
//GO(slaeda_, 
//GO(slaein_, 
//GO(slaev2_, 
//GO(slaexc_, 
//GO(slag2_, 
GO(slag2d_, vFppppppp)
//GO(slags2_, 
//GO(slagtf_, 
//GO(slagtm_, 
//GO(slagts_, 
//GO(slagv2_, 
//GO(slahqr_, 
//GO(slahr2_, 
//GO(slahrd_, 
//GO(slaic1_, 
//GO(slaisnan_, 
//GO(slaln2_, 
//GO(slals0_, 
//GO(slalsa_, 
//GO(slalsd_, 
//GO(slamc3_, 
GO(slamch_, fFpL)
//GO(slamrg_, 
//GO(slamswlq_, 
//GO(slamtsqr_, 
//GO(slaneg_, 
GO(slangb_, fFpppppppL)
GO(slange_, fFppppppL)
GO(slangt_, fFpppppL)
GO(slanhs_, fFpppppL)
GO(slansb_, fFpppppppLL)
//GO(slansf_, 
GO(slansp_, fFpppppLL)
GO(slanst_, fFppppL)
GO(slansy_, fFppppppLL)
GO(slantb_, fFppppppppLLL)
GO(slantp_, fFppppppLLL)
GO(slantr_, fFppppppppLLL)
//GO(slanv2_, 
//GO(slaorhr_col_getrfnp_, 
//GO(slaorhr_col_getrfnp2_, 
//GO(slapll_, 
GO(slapmr_, vFpppppp)
GO(slapmt_, vFpppppp)
GO(slapy2_, fFpp)
GO(slapy3_, fFppp)
//GO(slaqgb_, 
//GO(slaqge_, 
//GO(slaqp2_, 
//GO(slaqp2rk_, 
//GO(slaqp3rk_, 
//GO(slaqps_, 
//GO(slaqr0_, 
//GO(slaqr1_, 
//GO(slaqr2_, 
//GO(slaqr3_, 
//GO(slaqr4_, 
//GO(slaqr5_, 
//GO(slaqsb_, 
//GO(slaqsp_, 
//GO(slaqsy_, 
//GO(slaqtr_, 
//GO(slaqz0_, 
//GO(slaqz1_, 
//GO(slaqz2_, 
//GO(slaqz3_, 
//GO(slaqz4_, 
//GO(slar1v_, 
//GO(slar2v_, 
GO(slarf_, vFpppppppppL)
//GO(slarf1f_, 
//GO(slarf1l_, 
GO(slarfb_, vFpppppppppppppppLLLL)
//GO(slarfb_gett_, 
GO(slarfg_, vFppppp)
//GO(slarfgp_, 
GO(slarft_, vFpppppppppLL)
GO(slarfx_, vFppppppppL)
//GO(slarfy_, 
//GO(slargv_, 
//GO(slarmm_, 
GO(slarnv_, vFpppp)
//GO(slarra_, 
//GO(slarrb_, 
//GO(slarrc_, 
//GO(slarrd_, 
//GO(slarre_, 
//GO(slarrf_, 
//GO(slarrj_, 
//GO(slarrk_, 
//GO(slarrr_, 
//GO(slarrv_, 
//GO(slartg_, 
GO(slartgp_, vFppppp)
GO(slartgs_, vFppppp)
//GO(slartv_, 
//GO(slaruv_, 
//GO(slarz_, 
//GO(slarzb_, 
//GO(slarzt_, 
//GO(slas2_, 
GO(slascl_, vFppppppppppL)
//GO(slasd0_, 
//GO(slasd1_, 
//GO(slasd2_, 
//GO(slasd3_, 
//GO(slasd4_, 
//GO(slasd5_, 
//GO(slasd6_, 
//GO(slasd7_, 
//GO(slasd8_, 
//GO(slasda_, 
//GO(slasdq_, 
//GO(slasdt_, 
GO(slaset_, vFpppppppL)
//GO(slasq1_, 
//GO(slasq2_, 
//GO(slasq3_, 
//GO(slasq4_, 
//GO(slasq5_, 
//GO(slasq6_, 
//GO(slasr_, 
GO(slasrt_, vFppppL)
GO(slassq_, vFppppp)
//GO(slasv2_, 
//GO(slaswlq_, 
GO(slaswp_, vFppppppp)
//GO(slasy2_, 
//GO(slasyf_, 
//GO(slasyf_aa_, 
//GO(slasyf_rk_, 
//GO(slasyf_rook_, 
//GO(slatbs_, 
//GO(slatdf_, 
//GO(slatps_, 
//GO(slatrd_, 
//GO(slatrs_, 
//GO(slatrs3_, 
//GO(slatrz_, 
//GO(slatsqr_, 
//GO(slatzm_, 
//GO(slauu2_, 
GO(slauum_, vFpppppL)
GO(sopgtr_, vFppppppppL)
GO(sopmtr_, vFpppppppppppLLL)
GO(sorbdb_, vFppppppppppppppppppppppLL)
//GO(sorbdb1_, 
//GO(sorbdb2_, 
//GO(sorbdb3_, 
//GO(sorbdb4_, 
//GO(sorbdb5_, 
//GO(sorbdb6_, 
GO(sorcsd_, vFppppppppppppppppppppppppppppppLLLLLL)
GO(sorcsd2by1_, vFpppppppppppppppppppppLLL)
//GO(sorg2l_, 
//GO(sorg2r_, 
GO(sorgbr_, vFppppppppppL)
GO(sorghr_, vFppppppppp)
//GO(sorgl2_, 
GO(sorglq_, vFppppppppp)
GO(sorgql_, vFppppppppp)
GO(sorgqr_, vFppppppppp)
//GO(sorgr2_, 
GO(sorgrq_, vFppppppppp)
GO(sorgtr_, vFppppppppL)
//GO(sorgtsqr_, 
GO(sorgtsqr_row_, vFppppppppppp)
GO(sorhr_col_, vFppppppppp)
//GO(sorm22_, 
//GO(sorm2l_, 
//GO(sorm2r_, 
GO(sormbr_, vFppppppppppppppLLL)
GO(sormhr_, vFppppppppppppppLL)
//GO(sorml2_, 
GO(sormlq_, vFpppppppppppppLL)
GO(sormql_, vFpppppppppppppLL)
GO(sormqr_, vFpppppppppppppLL)
//GO(sormr2_, 
//GO(sormr3_, 
GO(sormrq_, vFpppppppppppppLL)
GO(sormrz_, vFppppppppppppppLL)
GO(sormtr_, vFpppppppppppppLLL)
GO(spbcon_, vFppppppppppL)
GO(spbequ_, vFpppppppppL)
GO(spbrfs_, vFpppppppppppppppppL)
GO(spbstf_, vFppppppL)
GO(spbsv_, vFpppppppppL)
GO(spbsvx_, vFpppppppppppppppppppppLLL)
//GO(spbtf2_, 
GO(spbtrf_, vFppppppL)
GO(spbtrs_, vFpppppppppL)
GO(spftrf_, vFpppppLL)
GO(spftri_, vFpppppLL)
GO(spftrs_, vFppppppppLL)
GO(spocon_, vFpppppppppL)
GO(spoequ_, vFppppppp)
GO(spoequb_, vFppppppp)
GO(sporfs_, vFppppppppppppppppL)
GO(sposv_, vFppppppppL)
GO(sposvx_, vFppppppppppppppppppppLLL)
GO(spotf2_, vFpppppL)
GO(spotrf_, vFpppppL)
GO(spotrf2_, vFpppppL)
GO(spotri_, vFpppppL)
GO(spotrs_, vFppppppppL)
GO(sppcon_, vFppppppppL)
GO(sppequ_, vFpppppppL)
GO(spprfs_, vFppppppppppppppL)
GO(sppsv_, vFpppppppL)
GO(sppsvx_, vFppppppppppppppppppLLL)
GO(spptrf_, vFppppL)
GO(spptri_, vFppppL)
GO(spptrs_, vFpppppppL)
//GO(spstf2_, 
GO(spstrf_, vFpppppppppL)
GO(sptcon_, vFppppppp)
GO(spteqr_, vFppppppppL)
GO(sptrfs_, vFpppppppppppppp)
GO(sptsv_, vFppppppp)
GO(sptsvx_, vFppppppppppppppppL)
GO(spttrf_, vFpppp)
GO(spttrs_, vFppppppp)
//GO(sptts2_, 
//GO(sroundup_lwork_, 
//GO(srscl_, 
//GO(ssb2st_kernels_, 
GO(ssbev_, vFpppppppppppLL)
GO(ssbev_2stage_, vFppppppppppppLL)
GO(ssbevd_, vFppppppppppppppLL)
GO(ssbevd_2stage_, vFppppppppppppppLL)
GO(ssbevx_, vFppppppppppppppppppppppLLL)
GO(ssbevx_2stage_, vFpppppppppppppppppppppppLLL)
GO(ssbgst_, vFpppppppppppppLL)
GO(ssbgv_, vFppppppppppppppLL)
GO(ssbgvd_, vFpppppppppppppppppLL)
GO(ssbgvx_, vFpppppppppppppppppppppppppLLL)
GO(ssbtrd_, vFppppppppppppLL)
GO(ssfrk_, vFppppppppppLLL)
GO(sspcon_, vFpppppppppL)
GO(sspev_, vFpppppppppLL)
GO(sspevd_, vFppppppppppppLL)
GO(sspevx_, vFppppppppppppppppppLLL)
GO(sspgst_, vFppppppL)
GO(sspgv_, vFpppppppppppLL)
GO(sspgvd_, vFppppppppppppppLL)
GO(sspgvx_, vFppppppppppppppppppppLLL)
GO(ssprfs_, vFpppppppppppppppL)
GO(sspsv_, vFppppppppL)
GO(sspsvx_, vFpppppppppppppppppLL)
GO(ssptrd_, vFpppppppL)
GO(ssptrf_, vFpppppL)
GO(ssptri_, vFppppppL)
GO(ssptrs_, vFppppppppL)
GO(sstebz_, vFppppppppppppppppppLL)
GO(sstedc_, vFpppppppppppL)
GO(sstegr_, vFppppppppppppppppppppLL)
GO(sstein_, vFppppppppppppp)
GO(sstemr_, vFpppppppppppppppppppppLL)
GO(ssteqr_, vFppppppppL)
GO(ssterf_, vFpppp)
GO(sstev_, vFppppppppL)
GO(sstevd_, vFpppppppppppL)
GO(sstevr_, vFppppppppppppppppppppLL)
GO(sstevx_, vFppppppppppppppppppLL)
GO(ssycon_, vFppppppppppL)
GO(ssycon_3_, vFpppppppppppL)
//GO(ssycon_rook_, 
GO(ssyconv_, vFppppppppLL)
//GO(ssyconvf_, 
//GO(ssyconvf_rook_, 
GO(ssyequb_, vFpppppppppL)
GO(ssyev_, vFpppppppppLL)
GO(ssyev_2stage_, vFpppppppppLL)
GO(ssyevd_, vFpppppppppppLL)
GO(ssyevd_2stage_, vFpppppppppppLL)
GO(ssyevr_, vFpppppppppppppppppppppLLL)
GO(ssyevr_2stage_, vFpppppppppppppppppppppLLL)
GO(ssyevx_, vFppppppppppppppppppppLLL)
GO(ssyevx_2stage_, vFppppppppppppppppppppLLL)
//GO(ssygs2_, 
GO(ssygst_, vFppppppppL)
GO(ssygv_, vFppppppppppppLL)
GO(ssygv_2stage_, vFppppppppppppLL)
GO(ssygvd_, vFppppppppppppppLL)
GO(ssygvx_, vFpppppppppppppppppppppppLLL)
GO(ssyrfs_, vFpppppppppppppppppL)
GO(ssysv_, vFpppppppppppL)
GO(ssysv_aa_, vFpppppppppppL)
GO(ssysv_aa_2stage_, vFppppppppppppppL)
GO(ssysv_rk_, vFppppppppppppL)
GO(ssysv_rook_, vFpppppppppppL)
GO(ssysvx_, vFppppppppppppppppppppLL)
GO(ssyswapr_, vFppppppL)
//GO(ssytd2_, 
//GO(ssytf2_, 
//GO(ssytf2_rk_, 
//GO(ssytf2_rook_, 
GO(ssytrd_, vFppppppppppL)
GO(ssytrd_2stage_, vFpppppppppppppLL)
//GO(ssytrd_sb2st_, 
//GO(ssytrd_sy2sb_, 
GO(ssytrf_, vFppppppppL)
GO(ssytrf_aa_, vFppppppppL)
GO(ssytrf_aa_2stage_, vFpppppppppppL)
GO(ssytrf_rk_, vFpppppppppL)
GO(ssytrf_rook_, vFppppppppL)
GO(ssytri_, vFpppppppL)
GO(ssytri2_, vFppppppppL)
GO(ssytri2x_, vFppppppppL)
GO(ssytri_3_, vFpppppppppL)
//GO(ssytri_3x_, 
//GO(ssytri_rook_, 
GO(ssytrs_, vFpppppppppL)
GO(ssytrs2_, vFppppppppppL)
GO(ssytrs_3_, vFppppppppppL)
GO(ssytrs_aa_, vFpppppppppppL)
GO(ssytrs_aa_2stage_, vFppppppppppppL)
GO(ssytrs_rook_, vFpppppppppL)
GO(stbcon_, vFpppppppppppLLL)
GO(stbrfs_, vFpppppppppppppppppLLL)
GO(stbtrs_, vFpppppppppppLLL)
GO(stfsm_, vFpppppppppppLLLLL)
GO(stftri_, vFppppppLLL)
GO(stfttp_, vFppppppLL)
GO(stfttr_, vFpppppppLL)
GO(stgevc_, vFppppppppppppppppLL)
//GO(stgex2_, 
GO(stgexc_, vFpppppppppppppppp)
GO(stgsen_, vFppppppppppppppppppppppppp)
GO(stgsja_, vFpppppppppppppppppppppppppLLL)
GO(stgsna_, vFppppppppppppppppppppLL)
//GO(stgsy2_, 
GO(stgsyl_, vFppppppppppppppppppppppL)
GO(stpcon_, vFpppppppppLLL)
GO(stplqt_, vFpppppppppppp)
GO(stplqt2_, vFpppppppppp)
GO(stpmlqt_, vFpppppppppppppppppLL)
GO(stpmqrt_, vFpppppppppppppppppLL)
GO(stpqrt_, vFpppppppppppp)
GO(stpqrt2_, vFpppppppppp)
GO(stprfb_, vFppppppppppppppppppLLLL)
GO(stprfs_, vFpppppppppppppppLLL)
GO(stptri_, vFpppppLL)
GO(stptrs_, vFpppppppppLLL)
GO(stpttf_, vFppppppLL)
GO(stpttr_, vFppppppL)
GO(strcon_, vFppppppppppLLL)
GO(strevc_, vFppppppppppppppLL)
GO(strevc3_, vFpppppppppppppppLL)
GO(strexc_, vFppppppppppL)
GO(strrfs_, vFppppppppppppppppLLL)
GO(strsen_, vFppppppppppppppppppLL)
GO(strsna_, vFppppppppppppppppppLL)
GO(strsyl_, vFpppppppppppppLL)
GO(strsyl3_, vFpppppppppppppppppLL)
//GO(strti2_, 
GO(strtri_, vFppppppLL)
GO(strtrs_, vFppppppppppLLL)
GO(strttf_, vFpppppppLL)
GO(strttp_, vFppppppL)
//GO(stzrqf_, 
GO(stzrzf_, vFpppppppp)
//GO(__xerbla, 
//GO(xerbla_array_, 
GO(zbbcsd_, vFpppppppppppppppppppppppppppppLLLLL)
GO(zbdsqr_, vFpppppppppppppppL)
GO(zcgesv_, vFpppppppppppppp)
GO(zcposv_, vFppppppppppppppL)
//GO(zdrscl_, 
GO(zgbbrd_, vFpppppppppppppppppppL)
GO(zgbcon_, vFppppppppppppL)
GO(zgbequ_, vFpppppppppppp)
GO(zgbequb_, vFpppppppppppp)
GO(zgbrfs_, vFpppppppppppppppppppL)
GO(zgbsv_, vFpppppppppp)
GO(zgbsvx_, vFppppppppppppppppppppppppLLL)
//GO(zgbtf2_, 
GO(zgbtrf_, vFpppppppp)
GO(zgbtrs_, vFpppppppppppL)
GO(zgebak_, vFppppppppppLL)
GO(zgebal_, vFppppppppL)
//GO(zgebd2_, 
GO(zgebrd_, vFppppppppppp)
GO(zgecon_, vFpppppppppL)
GO(zgedmd_, vFpppppppppppppppppppppppppppppppLLLL)
GO(zgedmdq_, vFpppppppppppppppppppppppppppppppppppLLLLLL)
GO(zgeequ_, vFpppppppppp)
GO(zgeequb_, vFpppppppppp)
//GOM(zgees_, vFEpppppppppppppppLL)
//GOM(zgeesx_, vFEppppppppppppppppppLLL)
GO(zgeev_, vFppppppppppppppLL)
GO(zgeevx_, vFppppppppppppppppppppppLLLL)
//GO(zgegs_, 
//GO(zgegv_, 
//GO(zgehd2_, 
GO(zgehrd_, vFppppppppp)
GO(zgejsv_, vFpppppppppppppppppppppLLLLLL)
GO(zgelq_, vFppppppppp)
GO(zgelq2_, vFppppppp)
GO(zgelqf_, vFpppppppp)
//GO(zgelqs_, 
//GO(zgelqt_, 
//GO(zgelqt3_, 
GO(zgels_, vFpppppppppppL)
GO(zgelsd_, vFppppppppppppppp)
GO(zgelss_, vFpppppppppppppp)
//GO(zgelst_, 
//GO(zgelsx_, 
GO(zgelsy_, vFpppppppppppppp)
GO(zgemlq_, vFppppppppppppppLL)
//GO(zgemlqt_, 
GO(zgemqr_, vFppppppppppppppLL)
GO(zgemqrt_, vFppppppppppppppLL)
GO(zgeql2_, vFppppppp)
GO(zgeqlf_, vFpppppppp)
GO(zgeqp3_, vFpppppppppp)
//GO(zgeqp3rk_, 
GO(zgeqpf_, vFppppppppp)
GO(zgeqr_, vFppppppppp)
GO(zgeqr2_, vFppppppp)
//GO(zgeqr2p_, 
GO(zgeqrf_, vFpppppppp)
GO(zgeqrfp_, vFpppppppp)
//GO(zgeqrs_, 
GO(zgeqrt_, vFppppppppp)
GO(zgeqrt2_, vFppppppp)
GO(zgeqrt3_, vFppppppp)
GO(zgerfs_, vFpppppppppppppppppL)
GO(zgerq2_, vFppppppp)
GO(zgerqf_, vFpppppppp)
//GO(zgesc2_, 
GO(zgesdd_, vFpppppppppppppppL)
GO(zgesv_, vFpppppppp)
GO(zgesvd_, vFpppppppppppppppLL)
GO(zgesvdq_, vFppppppppppppppppppppppLLLLL)
GO(zgesvdx_, vFppppppppppppppppppppppLLL)
GO(zgesvj_, vFppppppppppppppppLLL)
GO(zgesvx_, vFppppppppppppppppppppppLLL)
//GO(zgetc2_, 
GO(zgetf2_, vFpppppp)
GO(zgetrf_, vFpppppp)
GO(zgetrf2_, vFpppppp)
GO(zgetri_, vFppppppp)
GO(zgetrs_, vFpppppppppL)
GO(zgetsls_, vFpppppppppppL)
GO(zgetsqrhrt_, vFpppppppppppp)
GO(zggbak_, vFpppppppppppLL)
GO(zggbal_, vFppppppppppppL)
//GOM(zgges_, vFEpppppppppppppppppppppLLL)
//GOM(zgges3_, vFEpppppppppppppppppppppLLL)
//GOM(zggesx_, vFEppppppppppppppppppppppppppLLLL)
GO(zggev_, vFpppppppppppppppppLL)
GO(zggev3_, vFpppppppppppppppppLL)
GO(zggevx_, vFpppppppppppppppppppppppppppppLLLL)
GO(zggglm_, vFppppppppppppp)
GO(zgghd3_, vFppppppppppppppppLL)
GO(zgghrd_, vFppppppppppppppLL)
GO(zgglse_, vFppppppppppppp)
GO(zggqrf_, vFpppppppppppp)
GO(zggrqf_, vFpppppppppppp)
GO(zggsvd_, iFppppppppppppppppppppppppLLL)
GO(zggsvd3_, vFpppppppppppppppppppppppppLLL)
GO(zggsvp_, iFpppppppppppppppppppppppppLLL)
GO(zggsvp3_, vFppppppppppppppppppppppppppLLL)
//GO(zgsvj0_, 
//GO(zgsvj1_, 
GO(zgtcon_, vFpppppppppppL)
GO(zgtrfs_, vFppppppppppppppppppppL)
GO(zgtsv_, vFpppppppp)
GO(zgtsvx_, vFppppppppppppppppppppppLL)
GO(zgttrf_, vFppppppp)
GO(zgttrs_, vFpppppppppppL)
//GO(zgtts2_, 
//GO(zhb2st_kernels_, 
GO(zhbev_, vFppppppppppppLL)
GO(zhbev_2stage_, vFpppppppppppppLL)
GO(zhbevd_, vFppppppppppppppppLL)
GO(zhbevd_2stage_, vFppppppppppppppppLL)
GO(zhbevx_, vFpppppppppppppppppppppppLLL)
GO(zhbevx_2stage_, vFppppppppppppppppppppppppLLL)
GO(zhbgst_, vFppppppppppppppLL)
GO(zhbgv_, vFpppppppppppppppLL)
GO(zhbgvd_, vFpppppppppppppppppppLL)
GO(zhbgvx_, vFppppppppppppppppppppppppppLLL)
GO(zhbtrd_, vFppppppppppppLL)
GO(zhecon_, vFpppppppppL)
GO(zhecon_3_, vFppppppppppL)
//GO(zhecon_rook_, 
GO(zheequb_, vFpppppppppL)
GO(zheev_, vFppppppppppLL)
GO(zheev_2stage_, vFppppppppppLL)
GO(zheevd_, vFpppppppppppppLL)
GO(zheevd_2stage_, vFpppppppppppppLL)
GO(zheevr_, vFpppppppppppppppppppppppLLL)
GO(zheevr_2stage_, vFpppppppppppppppppppppppLLL)
GO(zheevx_, vFpppppppppppppppppppppLLL)
GO(zheevx_2stage_, vFpppppppppppppppppppppLLL)
//GO(zhegs2_, 
GO(zhegst_, vFppppppppL)
GO(zhegv_, vFpppppppppppppLL)
GO(zhegv_2stage_, vFpppppppppppppLL)
GO(zhegvd_, vFppppppppppppppppLL)
GO(zhegvx_, vFppppppppppppppppppppppppLLL)
GO(zherfs_, vFpppppppppppppppppL)
GO(zhesv_, vFpppppppppppL)
GO(zhesv_aa_, vFpppppppppppL)
GO(zhesv_aa_2stage_, vFppppppppppppppL)
GO(zhesv_rk_, vFppppppppppppL)
GO(zhesv_rook_, vFpppppppppppL)
GO(zhesvx_, vFppppppppppppppppppppLL)
GO(zheswapr_, vFppppppL)
//GO(zhetd2_, 
//GO(zhetf2_, 
//GO(zhetf2_rk_, 
//GO(zhetf2_rook_, 
GO(zhetrd_, vFppppppppppL)
GO(zhetrd_2stage_, vFpppppppppppppLL)
//GO(zhetrd_hb2st_, 
//GO(zhetrd_he2hb_, 
GO(zhetrf_, vFppppppppL)
GO(zhetrf_aa_, vFppppppppL)
GO(zhetrf_aa_2stage_, vFpppppppppppL)
GO(zhetrf_rk_, vFpppppppppL)
GO(zhetrf_rook_, vFppppppppL)
GO(zhetri_, vFpppppppL)
GO(zhetri2_, vFppppppppL)
GO(zhetri2x_, vFppppppppL)
GO(zhetri_3_, vFpppppppppL)
//GO(zhetri_3x_, 
//GO(zhetri_rook_, 
GO(zhetrs_, vFpppppppppL)
GO(zhetrs2_, vFppppppppppL)
GO(zhetrs_3_, vFppppppppppL)
GO(zhetrs_aa_, vFpppppppppppL)
GO(zhetrs_aa_2stage_, vFppppppppppppL)
GO(zhetrs_rook_, vFpppppppppL)
GO(zhfrk_, vFppppppppppLLL)
GO(zhgeqz_, vFppppppppppppppppppppLLL)
GO(zhpcon_, vFppppppppL)
GO(zhpev_, vFppppppppppLL)
GO(zhpevd_, vFppppppppppppppLL)
GO(zhpevx_, vFpppppppppppppppppppLLL)
GO(zhpgst_, vFppppppL)
GO(zhpgv_, vFppppppppppppLL)
GO(zhpgvd_, vFppppppppppppppppLL)
GO(zhpgvx_, vFpppppppppppppppppppppLLL)
GO(zhprfs_, vFpppppppppppppppL)
GO(zhpsv_, vFppppppppL)
GO(zhpsvx_, vFpppppppppppppppppLL)
GO(zhptrd_, vFpppppppL)
GO(zhptrf_, vFpppppL)
GO(zhptri_, vFppppppL)
GO(zhptrs_, vFppppppppL)
GO(zhsein_, vFpppppppppppppppppppLLL)
GO(zhseqr_, vFpppppppppppppLL)
//GO(zlabrd_, 
GO(zlacgv_, vFppp)
GO(zlacn2_, vFpppppp)
//GO(zlacon_, 
GO(zlacp2_, vFpppppppL)
GO(zlacpy_, vFpppppppL)
GO(zlacrm_, vFppppppppp)
//GO(zlacrt_, 
//GO(zladiv_, 
//GO(zlaed0_, 
//GO(zlaed7_, 
//GO(zlaed8_, 
//GO(zlaein_, 
//GO(zlaesy_, 
//GO(zlaev2_, 
GO(zlag2c_, vFppppppp)
//GO(zlags2_, 
//GO(zlagtm_, 
//GO(zlahef_, 
//GO(zlahef_aa_, 
//GO(zlahef_rk_, 
//GO(zlahef_rook_, 
//GO(zlahqr_, 
//GO(zlahr2_, 
//GO(zlahrd_, 
//GO(zlaic1_, 
//GO(zlals0_, 
//GO(zlalsa_, 
//GO(zlalsd_, 
//GO(zlamswlq_, 
//GO(zlamtsqr_, 
GO(zlangb_, dFpppppppL)
GO(zlange_, dFppppppL)
GO(zlangt_, dFpppppL)
GO(zlanhb_, dFpppppppLL)
GO(zlanhe_, dFppppppLL)
//GO(zlanhf_, 
GO(zlanhp_, dFpppppLL)
GO(zlanhs_, dFpppppL)
GO(zlanht_, dFppppL)
GO(zlansb_, dFpppppppLL)
GO(zlansp_, dFpppppLL)
GO(zlansy_, dFppppppLL)
GO(zlantb_, dFppppppppLLL)
GO(zlantp_, dFppppppLLL)
GO(zlantr_, dFppppppppLLL)
//GO(zlapll_, 
GO(zlapmr_, vFpppppp)
GO(zlapmt_, vFpppppp)
//GO(zlaqgb_, 
//GO(zlaqge_, 
//GO(zlaqhb_, 
//GO(zlaqhe_, 
//GO(zlaqhp_, 
//GO(zlaqp2_, 
//GO(zlaqp2rk_, 
//GO(zlaqp3rk_, 
//GO(zlaqps_, 
//GO(zlaqr0_, 
//GO(zlaqr1_, 
//GO(zlaqr2_, 
//GO(zlaqr3_, 
//GO(zlaqr4_, 
//GO(zlaqr5_, 
//GO(zlaqsb_, 
//GO(zlaqsp_, 
//GO(zlaqsy_, 
//GO(zlaqz0_, 
//GO(zlaqz1_, 
//GO(zlaqz2_, 
//GO(zlaqz3_, 
//GO(zlar1v_, 
//GO(zlar2v_, 
GO(zlarcm_, vFppppppppp)
GO(zlarf_, vFpppppppppL)
//GO(zlarf1f_, 
//GO(zlarf1l_, 
GO(zlarfb_, vFpppppppppppppppLLLL)
//GO(zlarfb_gett_, 
GO(zlarfg_, vFppppp)
//GO(zlarfgp_, 
GO(zlarft_, vFpppppppppLL)
GO(zlarfx_, vFppppppppL)
//GO(zlarfy_, 
//GO(zlargv_, 
GO(zlarnv_, vFpppp)
//GO(zlarrv_, 
//GO(zlartg_, 
//GO(zlartv_, 
//GO(zlarz_, 
//GO(zlarzb_, 
//GO(zlarzt_, 
GO(zlascl_, vFppppppppppL)
GO(zlaset_, vFpppppppL)
//GO(zlasr_, 
GO(zlassq_, vFppppp)
//GO(zlaswlq_, 
GO(zlaswp_, vFppppppp)
//GO(zlasyf_, 
//GO(zlasyf_aa_, 
//GO(zlasyf_rk_, 
//GO(zlasyf_rook_, 
//GO(zlat2c_, 
//GO(zlatbs_, 
//GO(zlatdf_, 
//GO(zlatps_, 
//GO(zlatrd_, 
//GO(zlatrs_, 
//GO(zlatrs3_, 
//GO(zlatrz_, 
//GO(zlatsqr_, 
//GO(zlatzm_, 
//GO(zlaunhr_col_getrfnp_, 
//GO(zlaunhr_col_getrfnp2_, 
//GO(zlauu2_, 
GO(zlauum_, vFpppppL)
GO(zpbcon_, vFppppppppppL)
GO(zpbequ_, vFpppppppppL)
GO(zpbrfs_, vFpppppppppppppppppL)
GO(zpbstf_, vFppppppL)
GO(zpbsv_, vFpppppppppL)
GO(zpbsvx_, vFpppppppppppppppppppppLLL)
//GO(zpbtf2_, 
GO(zpbtrf_, vFppppppL)
GO(zpbtrs_, vFpppppppppL)
GO(zpftrf_, vFpppppLL)
GO(zpftri_, vFpppppLL)
GO(zpftrs_, vFppppppppLL)
GO(zpocon_, vFpppppppppL)
GO(zpoequ_, vFppppppp)
GO(zpoequb_, vFppppppp)
GO(zporfs_, vFppppppppppppppppL)
GO(zposv_, vFppppppppL)
GO(zposvx_, vFppppppppppppppppppppLLL)
GO(zpotf2_, vFpppppL)
GO(zpotrf_, vFpppppL)
GO(zpotrf2_, vFpppppL)
GO(zpotri_, vFpppppL)
GO(zpotrs_, vFppppppppL)
GO(zppcon_, vFppppppppL)
GO(zppequ_, vFpppppppL)
GO(zpprfs_, vFppppppppppppppL)
GO(zppsv_, vFpppppppL)
GO(zppsvx_, vFppppppppppppppppppLLL)
GO(zpptrf_, vFppppL)
GO(zpptri_, vFppppL)
GO(zpptrs_, vFpppppppL)
//GO(zpstf2_, 
GO(zpstrf_, vFpppppppppL)
GO(zptcon_, vFppppppp)
GO(zpteqr_, vFppppppppL)
GO(zptrfs_, vFppppppppppppppppL)
GO(zptsv_, vFppppppp)
GO(zptsvx_, vFpppppppppppppppppL)
GO(zpttrf_, vFpppp)
GO(zpttrs_, vFppppppppL)
//GO(zptts2_, 
//GO(zrot_, 
//GO(zrscl_, 
GO(zspcon_, vFppppppppL)
//GO(zspmv_, 
//GO(zspr_, 
GO(zsprfs_, vFpppppppppppppppL)
GO(zspsv_, vFppppppppL)
GO(zspsvx_, vFpppppppppppppppppLL)
GO(zsptrf_, vFpppppL)
GO(zsptri_, vFppppppL)
GO(zsptrs_, vFppppppppL)
GO(zstedc_, vFpppppppppppppL)
GO(zstegr_, vFppppppppppppppppppppLL)
GO(zstein_, vFppppppppppppp)
GO(zstemr_, vFpppppppppppppppppppppLL)
GO(zsteqr_, vFppppppppL)
GO(zsycon_, vFpppppppppL)
GO(zsycon_3_, vFppppppppppL)
//GO(zsycon_rook_, 
GO(zsyconv_, vFppppppppLL)
//GO(zsyconvf_, 
//GO(zsyconvf_rook_, 
GO(zsyequb_, vFpppppppppL)
//GO(zsymv_, 
GO(zsyr_, vFpppppppL)
GO(zsyrfs_, vFpppppppppppppppppL)
GO(zsysv_, vFpppppppppppL)
GO(zsysv_aa_, vFpppppppppppL)
GO(zsysv_aa_2stage_, vFppppppppppppppL)
GO(zsysv_rk_, vFppppppppppppL)
GO(zsysv_rook_, vFpppppppppppL)
GO(zsysvx_, vFppppppppppppppppppppLL)
GO(zsyswapr_, vFppppppL)
//GO(zsytf2_, 
//GO(zsytf2_rk_, 
//GO(zsytf2_rook_, 
GO(zsytrf_, vFppppppppL)
GO(zsytrf_aa_, vFppppppppL)
GO(zsytrf_aa_2stage_, vFpppppppppppL)
GO(zsytrf_rk_, vFpppppppppL)
GO(zsytrf_rook_, vFppppppppL)
GO(zsytri_, vFpppppppL)
GO(zsytri2_, vFppppppppL)
GO(zsytri2x_, vFppppppppL)
GO(zsytri_3_, vFpppppppppL)
//GO(zsytri_3x_, 
//GO(zsytri_rook_, 
GO(zsytrs_, vFpppppppppL)
GO(zsytrs2_, vFppppppppppL)
GO(zsytrs_3_, vFppppppppppL)
GO(zsytrs_aa_, vFpppppppppppL)
GO(zsytrs_aa_2stage_, vFppppppppppppL)
GO(zsytrs_rook_, vFpppppppppL)
GO(ztbcon_, vFpppppppppppLLL)
GO(ztbrfs_, vFpppppppppppppppppLLL)
GO(ztbtrs_, vFpppppppppppLLL)
GO(ztfsm_, vFpppppppppppLLLLL)
GO(ztftri_, vFppppppLLL)
GO(ztfttp_, vFppppppLL)
GO(ztfttr_, vFpppppppLL)
GO(ztgevc_, vFpppppppppppppppppLL)
//GO(ztgex2_, 
GO(ztgexc_, vFpppppppppppppp)
GO(ztgsen_, vFpppppppppppppppppppppppp)
GO(ztgsja_, vFpppppppppppppppppppppppppLLL)
GO(ztgsna_, vFppppppppppppppppppppLL)
//GO(ztgsy2_, 
GO(ztgsyl_, vFppppppppppppppppppppppL)
GO(ztpcon_, vFpppppppppLLL)
GO(ztplqt_, vFpppppppppppp)
GO(ztplqt2_, vFpppppppppp)
GO(ztpmlqt_, vFpppppppppppppppppLL)
GO(ztpmqrt_, vFpppppppppppppppppLL)
GO(ztpqrt_, vFpppppppppppp)
GO(ztpqrt2_, vFpppppppppp)
GO(ztprfb_, vFppppppppppppppppppLLLL)
GO(ztprfs_, vFpppppppppppppppLLL)
GO(ztptri_, vFpppppLL)
GO(ztptrs_, vFpppppppppLLL)
GO(ztpttf_, vFppppppLL)
GO(ztpttr_, vFppppppL)
GO(ztrcon_, vFppppppppppLLL)
GO(ztrevc_, vFpppppppppppppppLL)
GO(ztrevc3_, vFpppppppppppppppppLL)
GO(ztrexc_, vFpppppppppL)
GO(ztrrfs_, vFppppppppppppppppLLL)
GO(ztrsen_, vFpppppppppppppppLL)
GO(ztrsna_, vFppppppppppppppppppLL)
GO(ztrsyl_, vFpppppppppppppLL)
GO(ztrsyl3_, vFpppppppppppppppLL)
//GO(ztrti2_, 
GO(ztrtri_, vFppppppLL)
GO(ztrtrs_, vFppppppppppLLL)
GO(ztrttf_, vFpppppppLL)
GO(ztrttp_, vFppppppL)
//GO(ztzrqf_, 
GO(ztzrzf_, vFpppppppp)
GO(zunbdb_, vFppppppppppppppppppppppLL)
//GO(zunbdb1_, 
//GO(zunbdb2_, 
//GO(zunbdb3_, 
//GO(zunbdb4_, 
//GO(zunbdb5_, 
//GO(zunbdb6_, 
GO(zuncsd_, vFppppppppppppppppppppppppppppppppLLLLLL)
GO(zuncsd2by1_, vFpppppppppppppppppppppppLLL)
//GO(zung2l_, 
//GO(zung2r_, 
GO(zungbr_, vFppppppppppL)
GO(zunghr_, vFppppppppp)
//GO(zungl2_, 
GO(zunglq_, vFppppppppp)
GO(zungql_, vFppppppppp)
GO(zungqr_, vFppppppppp)
//GO(zungr2_, 
GO(zungrq_, vFppppppppp)
GO(zungtr_, vFppppppppL)
//GO(zungtsqr_, 
GO(zungtsqr_row_, vFppppppppppp)
GO(zunhr_col_, vFppppppppp)
//GO(zunm22_, 
//GO(zunm2l_, 
//GO(zunm2r_, 
GO(zunmbr_, vFppppppppppppppLLL)
GO(zunmhr_, vFppppppppppppppLL)
//GO(zunml2_, 
GO(zunmlq_, vFpppppppppppppLL)
GO(zunmql_, vFpppppppppppppLL)
GO(zunmqr_, vFpppppppppppppLL)
//GO(zunmr2_, 
//GO(zunmr3_, 
GO(zunmrq_, vFpppppppppppppLL)
GO(zunmrz_, vFppppppppppppppLL)
GO(zunmtr_, vFpppppppppppppLLL)
GO(zupgtr_, vFppppppppL)
GO(zupmtr_, vFpppppppppppLLL)
