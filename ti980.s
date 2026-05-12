;
;	Test TI980 ops
;
;	Addressing modes
;	FIXME test with offsets
;
meep:
;	PC relative
	lda	meep
	lda	meep+12
	lda	meep-128
;	Immediate extended
	@lda	=1234
;	Immediate extended
	@lda	1234,7
;	Base relative B + D
foo:
	lda	foo,4
;	Indexed PC relative PC + X + SD
	lda	foo,2
;	Indirect PC relative (PC + D)
	lda	*foo+1
	lda	*foo,4
	lda	foo,4
	@lda	foo
;	Indirect Base relative (B + D)
	lda	*foo,1
	lda	*foo+2,5
	lda	foo,5
;	Indirect, indexed, PC relative
;	(PC + SD) + X
;	(PC + X + SD) depending on flags
	lda	*foo,6
	lda	*foo,2
	lda	foo,6
	@lda	foo,2	; always post indexed
;	Immediate
	lda	=foo
	lda	foo+4,7
;
;	Operations
;
	dld	=12345678        ; need 32bit working values in asm
	dld	foo
	lda	foo
	lde	=12
	ldm	*foo
	ldx	=-32
	lrf	foo
	dst	foo
	; FIXME stores to immediates not allowed
	srf 	foo
	sta	foo
	ste	foo,6
	stx	foo
	bix	bar
bar:
	brl	bar
	@brl	foo
	bru	foo
	; FIXME: idl is allowed alone
	idl	2
	lsr	foo
	ssb	foo
	add	foo
	add	5
	add	-3
	dad	foo
	div	foo
	dsb	foo
	imo	foo
	mpy	foo
	rad	e,a
	rco	e,e
	rde	l,a
	rin	m,m
	riv	x,s
	rsu	x,b
	sub	foo
	clc
	cpa	foo
	cpl	foo
	rca	x,b
	rcl	x,b
	dmt	foo
	seq
	sev	a
	sge
	sgt
	sle
	slt
	smi	e
	snc
	sne
	sno	x
	snv
	snz	x
	soc
	sod	a
	sov
	spl	a
	sse	7
	ssn	4
	sze	a
	ala	3
	ald	30
	ara	6
	ard	30
	cld	1
	cra	1
	crb	1
	crd	5
	cre	2
	crl	1
	crm	4
	crs	15
	crx	31
	lla	1
	lld	2
	lra	4
	lrd	6
	lto	2
	ltz	0
	nrm
	rto	31
	rtz	12
	and	foo
	ior	foo
	ran	a,x
	reo	a,x
	ror	a,x
	sabo	0
	sabz	7
	smbo	1,foo
	smbz	15,foo
	tabo	2
	tabz	5
	tmbo	1,foo
	tmbz	0,foo
	mvc
	rex	x,b
	rmo	x,b
	api	1,foo
	ati	3,11,foo
;	need to figure out a sane syntax
;	rds	7,x
;	wds	7,x
