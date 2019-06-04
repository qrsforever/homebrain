(defrule rul-start-homebrain
    ?f <- (start-homebrain)
  =>
    (bind ?c (create-rule-context rul-start-homebrain))
    (assert (scene (where room1)))
)
