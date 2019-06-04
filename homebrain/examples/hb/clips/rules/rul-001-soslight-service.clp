(defrule rul-001-soslight-service
    (datetime ?clock $?other)
    ?svr <- (service 
        (name ?name &:(eq ?name soslight))
        (ntime ?tm &:(> ?clock ?tm))
        (state ?st) (saves $?saves)
    )
    ?ins <- (object (is-a oic.d.light)
        (ID ?did &:(eq ?did ins-00a17a88-a01a-02a1-4af9))
        (value ?val ?val-old)
    )
  =>
    (bind ?c (create-rule-context rul-001-soslight-service))
    (if (eq ?st stop)
     then
        (retract ?svr)
        (bind ?sav (nth$ 1 $?saves)) 
        (send ?c act-control ?did value ?sav)
        (return)
    )
    (bind ?cur (send ?ins getData value))
    (bind ?ct (mod (random) 777))
    (bind ?nti (+ ?clock 1000))
    (if (eq ?st start)
     then
        (modify ?svr (ntime ?nti) (saves ?cur))
     else
        (modify ?svr (ntime ?nti) (state running))
    )
    (if (= ?cur 1)
     then
        (send ?c act-control ?did value 0)
     else
        (send ?c act-control ?did value 1)
        (send ?c act-control ?did ct ?ct)
    )
)
