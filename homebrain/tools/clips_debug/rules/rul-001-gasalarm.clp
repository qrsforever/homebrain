(defrule rul-001-gasalarm
    ?sct <- (object (is-a SceneContext)
         (where room0) (target gasalarm) (action ?action)
         (services $?svrs)
    )
    ?light <- (object (is-a oic.d.light)
        (ID ?did &:(eq ?did ins-00a17a88-a01a-03aa-0aa4)))
  =>
    (bind ?c (create-rule-context rul-001-gasalarm))
    (logi "scene gasalarm, action: " ?action)
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
        (bind ?etime (+ 30000 (nth$ 1 (now))))
        (bind ?hue 50)
        (bind ?sat 250)
        (assert (start-service ?sct alarmlight:1 ?etime ?did ?hue ?sat))
    )
)
