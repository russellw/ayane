; unsat
(declare-fun a () Int)
(assert (=  a 5))
(assert
	(let ((x a))
		(=  x 6)
	)
)
(check-sat)
