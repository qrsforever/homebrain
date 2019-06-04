(defrule rul-001-leavehome
    ?sct <- (object (is-a SceneContext)
         (where ?room) (target leavehome) (action ?action)
         (services $?svrs)
    )
    ?light2 <- (object (is-a oic.d.light)
        (ID ?did2 &:(eq ?did2 ins-00a17a88-a01a-03aa-0aa4)))
    ?light3 <- (object (is-a oic.d.light)
        (ID ?did3 &:(eq ?did3 ins-00a17a88-a01a-03a6-4a67)))
  =>
    (bind ?c (create-rule-context rul-001-leavehome))
    (logi "scene leavehome, action: " ?action)
    (if (eq ?action exit)
     then
        (return)
    )
    (if (eq ?action enter)
     then
        (send ?c act-control ?did2 value 0)
        (send ?c act-control ?did3 value 0)
    )
)
