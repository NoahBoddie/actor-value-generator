scriptName CommonLibSSESampleTest extends Quest

Actor property Player auto
Int iterations = 0

event OnInit()
    Utility.Wait(0.1)
    HitCounter.StartCounting(Player)
    RegisterForSingleUpdate(30.0)
endEvent

event OnUpdate()
    iterations = iterations + 1
    Debug.Notification("Player has been hit " + HitCounter.GetCount(Player) + " times.")
    if iterations < 10
        RegisterForSingleUpdate(30.0)
    else
        HitCounter.StopCounting(Player)
    endIf
endEvent
