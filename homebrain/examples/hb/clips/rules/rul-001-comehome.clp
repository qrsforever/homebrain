(defrule rul-001-comehome
    ?f <- (scene (where room1) (target comehome) (action ?action))
  =>
    (bind ?c (create-rule-context rul-001-comehome))
    (if (eq ?action exit)
     then
        (printout t "scene comehome exit" crlf)
        (assert (stop-service autolight))
        (return)
    )
    (printout t "scene comehome enter" crlf)
    (assert (start-service autolight ins-00124b00-12d2-6263-d0d0 ins-00a17a88-a01a-03a3-6a2b 30)
    (send ?c act-control ins-00a17a88-a01a-02a1-4af9 value 1)
)
