(set-logic QF_UF)
(set-info :status sat)
(declare-sort U 0)
(declare-fun x () U)
(declare-fun y () U)
(declare-fun a () Bool)
(assert (not (= x (ite a (ite a x y) (ite (not a) y x)))))
(check-sat)
