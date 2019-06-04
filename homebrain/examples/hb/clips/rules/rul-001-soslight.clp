(defrule rul-001-soslight
    ?f <- (scene (where room1) (target soslight) (action ?action) (follow ?follow))
  =>
    (bind ?c (create-rule-context rul-001-soslight))
    (if (eq ?action exit)
     then
        (printout t "scene soslight exit" crlf)
        (assert (stop-service soslight))
        (return)
    )
    (printout t "scene soslight enter" crlf)
    (assert (start-service soslight))
)
