(defrule rul-001-sleep
    ?sct <- (object (is-a SceneContext)
         (where room1) (target sleep) (action ?action)
         (services $?svrs)
    )
    ?light2 <- (object (is-a oic.d.light)
        (ID ?did2 &:(eq ?did2 ins-00a17a88-a01a-03aa-0aa4)))
    ?light3 <- (object (is-a oic.d.light)
        (ID ?did3 &:(eq ?did3 ins-00a17a88-a01a-03a6-4a67)))
  =>
    (bind ?c (create-rule-context rul-001-sleep))
    (logi "scene sleep, action: " ?action)
    (if (eq ?action exit)
     then
        (foreach ?sname (create$ $?svrs)
            (logd "stop service: " ?sname)
            (assert (stop-service ?sname))
        )
        (return)
    )
    (if (eq ?action enter)
     then
        (bind ?count 8)
        (bind ?timems 8000)
        (assert (start-service ?sct gradlight-down:2 ?did2 ?count ?timems))
        (assert (start-service ?sct gradlight-down:3 ?did3 ?count ?timems))
    )
)