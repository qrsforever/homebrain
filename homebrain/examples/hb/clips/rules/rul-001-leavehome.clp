(defrule rul-001-leavehome
    ?f <- (scene (where room1) (target leavehome) (action ?action))
  =>
    (bind ?c (create-rule-context rul-001-leavehome))
    (if (eq ?action exit)
     then
        (printout t "scene leavehome exit" crlf)
        (return)
    )
    (printout t "scene leavehome enter" crlf)
    (send ?c act-control ins-00a17a88-a01a-02a1-4af9 value 0)
)
