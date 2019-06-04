(defrule rul-001-tvstate
    ?tv <- (object (is-a oic.d.letv)
        (power ?sw ?sw-old)
    )
  =>
    (bind ?c (create-rule-context rul-001-tvstate))
    (logi "tvstate:" ?sw)
    (if (= ?sw 1)
     then
        (assert (switch-scene room1 entermovie))
        (return)
    )
    (if (= ?sw 0)
     then
        (assert (switch-scene room1 exitmovie))
        (return)
    )
)
