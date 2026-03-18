-- ============================================================
-- Silent Assassins ESP + Hitbox
-- Game ID: 103854444055060
-- KHÔNG có getkey, KHÔNG có godmode
-- Toggle: RightShift (ESP) | RightControl (Hitbox) | Nút UI
-- ============================================================

local Players          = game:GetService("Players")
local RunService       = game:GetService("RunService")
local UserInputService = game:GetService("UserInputService")

local lp        = Players.LocalPlayer
local ESP_ON    = true
local HITBOX_ON = false
local TAG       = "SA_ESP_" .. math.random(1e5, 9e5)
local HB_TAG    = "SA_HB_"  .. math.random(1e4, 9e4)
local TARGET_ON  = false
local TARGET_PLAYER = nil  -- player đang bị target


-- ============================================================
-- CONFIG
-- ============================================================
local CFG = {
    MaxDist    = 1200,
    TextSize   = 13,
    BoxColor   = Color3.fromRGB(80,  255, 120),
    AlertColor = Color3.fromRGB(255, 60,  60),
    BgAlpha    = 0.45,
    HitboxSize = Vector3.new(9999, 9999, 9999),
}

-- ============================================================
-- DETECT EXPOSED
-- ============================================================
local function isExposed(player)
    local char = player.Character
    if not char then return false end
    if char:FindFirstChild("Exposed")     then return true end
    if char:FindFirstChild("Visible")     then return true end
    if char:FindFirstChild("Detected")    then return true end
    if char:FindFirstChild("IsAttacking") then return true end
    if char:FindFirstChildOfClass("SelectionBox") then return true end
    if char:FindFirstChildOfClass("Highlight")    then return true end
    return false
end

-- ============================================================
-- ESP
-- ============================================================
local function createESP(player)
    local char = player.Character
    if not char then return end
    local hrp = char:FindFirstChild("HumanoidRootPart")
    if not hrp then return end

    local old = hrp:FindFirstChild(TAG)
    if old then old:Destroy() end

    local bb = Instance.new("BillboardGui")
    bb.Name=TAG; bb.AlwaysOnTop=true
    bb.Size=UDim2.new(0,180,0,52); bb.StudsOffset=Vector3.new(0,3.5,0)
    bb.MaxDistance=CFG.MaxDist; bb.LightInfluence=0
    bb.Enabled=ESP_ON; bb.Parent=hrp

    local frame=Instance.new("Frame",bb)
    frame.Size=UDim2.new(1,0,1,0)
    frame.BackgroundColor3=Color3.new(0,0,0)
    frame.BackgroundTransparency=CFG.BgAlpha
    frame.BorderSizePixel=0
    Instance.new("UICorner",frame).CornerRadius=UDim.new(0,6)

    local stroke=Instance.new("UIStroke",frame)
    stroke.Color=CFG.BoxColor; stroke.Thickness=1.8

    local nameLbl=Instance.new("TextLabel",frame)
    nameLbl.Name="NameLbl"
    nameLbl.Size=UDim2.new(1,-6,0.55,0); nameLbl.Position=UDim2.new(0,3,0,3)
    nameLbl.BackgroundTransparency=1
    nameLbl.Text="⚔ "..player.DisplayName
    nameLbl.TextColor3=CFG.BoxColor; nameLbl.TextSize=CFG.TextSize
    nameLbl.Font=Enum.Font.GothamBold
    nameLbl.TextXAlignment=Enum.TextXAlignment.Center
    nameLbl.TextStrokeTransparency=0.4

    local distLbl=Instance.new("TextLabel",frame)
    distLbl.Name="DistLbl"
    distLbl.Size=UDim2.new(1,-6,0.4,0); distLbl.Position=UDim2.new(0,3,0.58,0)
    distLbl.BackgroundTransparency=1; distLbl.Text="..."
    distLbl.TextColor3=Color3.fromRGB(200,200,200)
    distLbl.TextSize=CFG.TextSize-2; distLbl.Font=Enum.Font.Gotham
    distLbl.TextXAlignment=Enum.TextXAlignment.Center
    distLbl.TextStrokeTransparency=0.5
end

local function updateESP(player)
    local char=player.Character
    if not char then return end
    local hrp=char:FindFirstChild("HumanoidRootPart")
    if not hrp then return end
    local bb=hrp:FindFirstChild(TAG)
    if not bb then createESP(player); return end

    local hum=char:FindFirstChildOfClass("Humanoid")
    if hum and hum.Health<=0 then bb.Enabled=false; return end

    local myHRP=lp.Character and lp.Character:FindFirstChild("HumanoidRootPart")
    if not myHRP then return end
    local dist=math.floor((hrp.Position-myHRP.Position).Magnitude)
    if dist>CFG.MaxDist then bb.Enabled=false; return end
    bb.Enabled=ESP_ON

    local exposed=isExposed(player)
    local color=exposed and CFG.AlertColor or CFG.BoxColor
    local icon=exposed and "🚨 " or "⚔ "

    local frame=bb:FindFirstChildOfClass("Frame")
    if not frame then return end
    local stroke=frame:FindFirstChildOfClass("UIStroke")
    if stroke then stroke.Color=color end
    local nl=frame:FindFirstChild("NameLbl")
    if nl then nl.Text=icon..player.DisplayName; nl.TextColor3=color end
    local dl=frame:FindFirstChild("DistLbl")
    if dl then dl.Text=tostring(dist).." studs" end
end

local function removeESP(player)
    local char=player.Character
    if not char then return end
    local hrp=char:FindFirstChild("HumanoidRootPart")
    if hrp then
        local bb=hrp:FindFirstChild(TAG)
        if bb then bb:Destroy() end
    end
end

-- ============================================================
-- HITBOX - set thẳng HRP.Size
-- ============================================================
local _origSizes = {}

local function expandHitbox(player)
    local char=player.Character
    if not char then return end
    local hrp=char:FindFirstChild("HumanoidRootPart")
    if not hrp then return end
    -- Lưu size gốc lần đầu
    if not _origSizes[player] then
        _origSizes[player] = hrp.Size
    end
    pcall(function()
        hrp.Size = CFG.HitboxSize
    end)
end

local function removeHitbox(player)
    local char=player.Character
    if not char then return end
    local hrp=char:FindFirstChild("HumanoidRootPart")
    if hrp and _origSizes[player] then
        pcall(function() hrp.Size = _origSizes[player] end)
    end
    _origSizes[player] = nil
end

local function removeAllHitboxes()
    for _,p in pairs(Players:GetPlayers()) do
        if p~=lp then pcall(removeHitbox,p) end
    end
    _origSizes = {}
end
-- ============================================================
-- KILL TARGET - dùng AttemptWeaponHit remote đã spy
-- ============================================================
-- Remote: ReplicatedStorage.Events.GameRemoteFunction
local _rem = nil
local function getRemote()
    if _rem then return _rem end
    pcall(function()
        _rem = game:GetService("ReplicatedStorage")
            :WaitForChild("Events", 5)
            :WaitForChild("GameRemoteFunction", 5)
    end)
    return _rem
end

-- Lấy tool đang cầm (dùng làm weapon arg)
local function getTool()
    local char = lp.Character
    if not char then return nil end
    return char:FindFirstChildOfClass("Tool")
end

local function killTarget()
    if not TARGET_PLAYER then return end
    local char = TARGET_PLAYER.Character
    if not char then return end
    local hrp = char:FindFirstChild("HumanoidRootPart")
    if not hrp then return end
    local hum = char:FindFirstChildOfClass("Humanoid")
    if not hum or hum.Health <= 0 then
        TARGET_PLAYER = nil
        TARGET_ON = false
        return
    end

    local myChar = lp.Character
    if not myChar then return end
    local myHRP = myChar:FindFirstChild("HumanoidRootPart")
    if not myHRP then return end

    local rem = getRemote()
    if not rem then return end

    local tool = getTool()
    local dist = (hrp.Position - myHRP.Position).Magnitude
    local dir  = dist > 0.01
        and (hrp.Position - myHRP.Position).Unit
        or  Vector3.new(0, 0, -1)

    -- Dùng vector3 thường thay vì vector.create (spy dùng vector.create nhưng Luau accept cả 2)
    local hitboxSz = Vector3.new(999, 999, 999)
    local hitboxOff= Vector3.new(0, 0, -1.5)

    local ok, err = pcall(function()
        rem:InvokeServer("AttemptWeaponHit",
            {
                attackCycleData = {
                    knockbackMul=1, slowMult=0.2,
                    slowTime=1.5, lungeMul=1, attackTime=0.65
                },
                knockback      = 50,
                shouldLock     = true,
                slowTime       = 1.5,
                shouldLunge    = true,
                isCritical     = false,
                weaponDefinition = {
                    attackCycle = {
                        ["1"] = {knockbackMul=1,   slowMult=0.2, slowTime=1.5, lungeMul=1,    attackTime=0.65},
                        ["2"] = {lungeMult=1,       slowMult=0.2, slowTime=1.5, knockbackMult=1,   attackTime=0.65},
                        ["3"] = {lungeMult=0.75,    slowMult=0.2, slowTime=1.5, knockbackMult=1.5,  attackTime=0.716},
                        ["4"] = {lungeMult=2.25,    slowMult=0.2, slowTime=1.5, knockbackMult=2.25, attackTime=0.983,
                                 hitboxOffsetAdd=Vector3.new(0,0,-1.5), hitboxSizeAdd=Vector3.new(0,0,3)},
                    },
                    attackOrder = {"1","2","3","4"},
                },
                attackCooldown = 0,
                shouldSlow     = true,
                lungeKnockback = 55,
                hitboxSize     = hitboxSz,  -- 999x999x999
                slowMult       = 0.2,
                cycleIndex     = (tick() % 4) + 1,  -- xoay vòng combo
                hitboxOffset   = hitboxOff,
                tool           = tool,  -- nil ok, server tự lấy
                damage         = math.huge,
            },
            {
                {
                    direction      = dir,
                    isClosestEnemy = true,
                    origin         = myHRP.Position,
                    enemyModel     = char,
                    distance       = dist,
                    knockback      = 50,
                }
            }
        )
    end)
    if not ok then
        warn("[KillTarget] Remote error:", err)
    end
end

-- Kill loop - spam tối đa, không delay
task.spawn(function()
    while true do
        if TARGET_ON and TARGET_PLAYER then
            -- Spam 5 lần liên tiếp mỗi frame
            for i = 1, 5 do
                pcall(killTarget)
            end
        end
        task.wait()  -- 1 frame (~0.016s) thay vì 0.05s
    end
end)



-- ============================================================
-- SETUP PLAYER
-- ============================================================
local function setupPlayer(player)
    if player==lp then return end
    if player.Character then
        createESP(player)
        if HITBOX_ON then expandHitbox(player) end
    end
    player.CharacterAdded:Connect(function()
        task.wait(0.5)
        createESP(player)
        if HITBOX_ON then task.wait(0.2); expandHitbox(player) end
    end)
    player.CharacterRemoving:Connect(function()
        task.wait(0.1)
        removeESP(player)
        removeHitbox(player)
    end)
end

for _,p in pairs(Players:GetPlayers()) do setupPlayer(p) end
Players.PlayerAdded:Connect(setupPlayer)
Players.PlayerRemoving:Connect(function(p)
    removeESP(p); removeHitbox(p)
end)

-- ============================================================
-- MAIN LOOP
-- ============================================================
RunService.RenderStepped:Connect(function()
    for _,p in pairs(Players:GetPlayers()) do
        if p~=lp then
            pcall(updateESP, p)
            if HITBOX_ON then pcall(expandHitbox, p) end
        end
    end
end)

-- ============================================================
-- UI - 2 NÚT KÉO ĐƯỢC
-- ============================================================
local gui=Instance.new("ScreenGui")
gui.Name="SA_Clean_GUI"; gui.ResetOnSpawn=false
gui.ZIndexBehavior=Enum.ZIndexBehavior.Sibling
gui.Parent=game.CoreGui

local container=Instance.new("Frame",gui)
container.Size=UDim2.new(0,160,0,122)
container.Position=UDim2.new(0,12,0.45,0)
container.BackgroundColor3=Color3.fromRGB(12,12,12)
container.BackgroundTransparency=0.25
container.BorderSizePixel=0; container.Active=true
Instance.new("UICorner",container).CornerRadius=UDim.new(0,10)
local cStr=Instance.new("UIStroke",container)
cStr.Color=Color3.fromRGB(55,55,55); cStr.Thickness=1

-- Drag
local drag,ds,sp=false,nil,nil
container.InputBegan:Connect(function(i)
    if i.UserInputType==Enum.UserInputType.MouseButton1
    or i.UserInputType==Enum.UserInputType.Touch then
        drag=true; ds=i.Position; sp=container.Position
    end
end)
container.InputChanged:Connect(function(i)
    if drag and(i.UserInputType==Enum.UserInputType.MouseMovement
    or i.UserInputType==Enum.UserInputType.Touch) then
        local d=i.Position-ds
        container.Position=UDim2.new(sp.X.Scale,sp.X.Offset+d.X,sp.Y.Scale,sp.Y.Offset+d.Y)
    end
end)
UserInputService.InputEnded:Connect(function(i)
    if i.UserInputType==Enum.UserInputType.MouseButton1
    or i.UserInputType==Enum.UserInputType.Touch then drag=false end
end)

local function makeBtn(txt,col,y)
    local b=Instance.new("TextButton",container)
    b.Size=UDim2.new(1,-12,0,30); b.Position=UDim2.new(0,6,0,y)
    b.BackgroundColor3=Color3.fromRGB(22,22,22); b.BorderSizePixel=0
    b.Text=txt; b.TextColor3=col; b.TextSize=13; b.Font=Enum.Font.GothamBold
    b.AutoButtonColor=false
    Instance.new("UICorner",b).CornerRadius=UDim.new(0,6)
    local s=Instance.new("UIStroke",b); s.Color=col; s.Thickness=1.3
    return b,s
end

local espBtn,espStr = makeBtn("ESP: ON",     Color3.fromRGB(80,255,120), 8)
local hbBtn, hbStr  = makeBtn("HITBOX: OFF", Color3.fromRGB(160,160,160), 46)

-- Toggle ESP
local function toggleESP()
    ESP_ON=not ESP_ON
    for _,p in pairs(Players:GetPlayers()) do
        if p~=lp and p.Character then
            local hrp=p.Character:FindFirstChild("HumanoidRootPart")
            if hrp then
                local bb=hrp:FindFirstChild(TAG)
                if bb then bb.Enabled=ESP_ON end
            end
        end
    end
    if ESP_ON then
        espBtn.Text="ESP: ON"; espBtn.TextColor3=Color3.fromRGB(80,255,120)
        espStr.Color=Color3.fromRGB(80,255,120)
    else
        espBtn.Text="ESP: OFF"; espBtn.TextColor3=Color3.fromRGB(255,80,80)
        espStr.Color=Color3.fromRGB(255,80,80)
    end
end

-- Toggle Hitbox
local function toggleHitbox()
    HITBOX_ON=not HITBOX_ON
    if HITBOX_ON then
        hbBtn.Text="HITBOX: ON"; hbBtn.TextColor3=Color3.fromRGB(255,100,100)
        hbStr.Color=Color3.fromRGB(255,100,100)
        for _,p in pairs(Players:GetPlayers()) do
            if p~=lp then pcall(expandHitbox,p) end
        end
        game:GetService("StarterGui"):SetCore("SendNotification",{
            Title="Silent Assassins",Text="✅ Hitbox BẬT (MAX)",Duration=2})
    else
        hbBtn.Text="HITBOX: OFF"; hbBtn.TextColor3=Color3.fromRGB(160,160,160)
        hbStr.Color=Color3.fromRGB(160,160,160)
        removeAllHitboxes()
        game:GetService("StarterGui"):SetCore("SendNotification",{
            Title="Silent Assassins",Text="❌ Hitbox TẮT",Duration=2})
    end
end

espBtn.MouseButton1Click:Connect(toggleESP)
hbBtn.MouseButton1Click:Connect(toggleHitbox)

-- ============================================================
-- TARGET SELECTOR UI - dropdown chọn player
-- ============================================================

-- Mở rộng container
container.Size = UDim2.new(0, 200, 0, 122)

local targetBtn, targetStr = makeBtn("🎯 TARGET: OFF", Color3.fromRGB(160,160,160), 84)

-- Panel dropdown chọn player
local dropPanel = Instance.new("Frame", gui)
dropPanel.Size             = UDim2.new(0, 200, 0, 8)
dropPanel.BackgroundColor3 = Color3.fromRGB(18, 18, 28)
dropPanel.BorderSizePixel  = 0
dropPanel.Visible          = false
dropPanel.ZIndex           = 20
dropPanel.ClipsDescendants = true
Instance.new("UICorner", dropPanel).CornerRadius = UDim.new(0, 8)
local dpStr = Instance.new("UIStroke", dropPanel)
dpStr.Color = Color3.fromRGB(255,80,80); dpStr.Thickness = 1.5

local dpLayout = Instance.new("UIListLayout", dropPanel)
dpLayout.Padding = UDim.new(0, 2)
local dpPad = Instance.new("UIPadding", dropPanel)
dpPad.PaddingTop    = UDim.new(0,4)
dpPad.PaddingBottom = UDim.new(0,4)
dpPad.PaddingLeft   = UDim.new(0,4)
dpPad.PaddingRight  = UDim.new(0,4)

local dropOpen = false

local function closeDropdown()
    dropOpen = false
    dropPanel.Visible = false
end

local function setTarget(player)
    TARGET_PLAYER = player
    TARGET_ON     = true
    targetBtn.Text       = "🎯 " .. player.DisplayName
    targetBtn.TextColor3 = Color3.fromRGB(255, 80, 80)
    targetStr.Color      = Color3.fromRGB(255, 80, 80)
    closeDropdown()
    game:GetService("StarterGui"):SetCore("SendNotification",{
        Title="Kill Target",
        Text="🎯 Đang kill: " .. player.DisplayName,
        Duration=2
    })
end

local function clearTarget()
    TARGET_PLAYER = nil
    TARGET_ON     = false
    targetBtn.Text       = "🎯 TARGET: OFF"
    targetBtn.TextColor3 = Color3.fromRGB(160,160,160)
    targetStr.Color      = Color3.fromRGB(160,160,160)
    closeDropdown()
end

local function buildDropdown()
    -- Xóa items cũ
    for _, v in pairs(dropPanel:GetChildren()) do
        if v:IsA("TextButton") then v:Destroy() end
    end

    local count = 0

    -- Nút "Tắt target"
    local offBtn = Instance.new("TextButton", dropPanel)
    offBtn.Size             = UDim2.new(1, 0, 0, 28)
    offBtn.BackgroundColor3 = Color3.fromRGB(50, 30, 30)
    offBtn.BorderSizePixel  = 0
    offBtn.Text             = "❌  Tắt target"
    offBtn.TextColor3       = Color3.fromRGB(200, 100, 100)
    offBtn.TextSize         = 12
    offBtn.Font             = Enum.Font.GothamBold
    offBtn.ZIndex           = 21
    offBtn.AutoButtonColor  = false
    Instance.new("UICorner", offBtn).CornerRadius = UDim.new(0,6)
    offBtn.MouseButton1Click:Connect(clearTarget)
    count = count + 1

    -- Các player trong server
    for _, p in pairs(Players:GetPlayers()) do
        if p ~= lp then
            local isTarget = (TARGET_PLAYER == p)
            local btn = Instance.new("TextButton", dropPanel)
            btn.Size             = UDim2.new(1, 0, 0, 28)
            btn.BackgroundColor3 = isTarget
                and Color3.fromRGB(60, 20, 20)
                or  Color3.fromRGB(25, 25, 38)
            btn.BorderSizePixel  = 0
            btn.Text             = (isTarget and "🎯 " or "👤 ") .. p.DisplayName
            btn.TextColor3       = isTarget
                and Color3.fromRGB(255, 100, 100)
                or  Color3.fromRGB(220, 220, 220)
            btn.TextSize         = 12
            btn.Font             = Enum.Font.GothamBold
            btn.ZIndex           = 21
            btn.AutoButtonColor  = false
            Instance.new("UICorner", btn).CornerRadius = UDim.new(0, 6)
            btn.MouseButton1Click:Connect(function()
                setTarget(p)
            end)
            count = count + 1
        end
    end

    -- Resize panel theo số items
    local itemH = 28
    local padding = 8 + (count - 1) * 2
    dropPanel.Size = UDim2.new(0, 200, 0, count * itemH + padding)
end

-- Toggle dropdown khi bấm nút target
targetBtn.MouseButton1Click:Connect(function()
    if dropOpen then
        closeDropdown()
    else
        buildDropdown()
        task.wait()  -- đợi 1 frame để tránh conflict
        local absPos  = container.AbsolutePosition
        local absSize = container.AbsoluteSize
        dropPanel.Position = UDim2.new(0, absPos.X, 0, absPos.Y + absSize.Y + 4)
        dropPanel.Visible  = true
        dropOpen = true
    end
end)

-- Đóng dropdown khi click ra ngoài (dùng GuiService thay vì InputBegan)
-- Không dùng InputBegan vì nó conflict với click nút trong dropdown
game:GetService("RunService").Heartbeat:Connect(function()
    if not dropOpen then return end
    -- Tự đóng sau 8 giây nếu không dùng
end)

-- Auto update tên nút khi target chết
task.spawn(function()
    while task.wait(0.5) do
        if TARGET_PLAYER then
            local char = TARGET_PLAYER.Character
            local hum  = char and char:FindFirstChildOfClass("Humanoid")
            if not char or (hum and hum.Health <= 0) then
                targetBtn.Text       = "🎯 " .. TARGET_PLAYER.DisplayName .. " (chết)"
                targetBtn.TextColor3 = Color3.fromRGB(255, 180, 0)
            end
        end
    end
end)



UserInputService.InputBegan:Connect(function(i,gpe)
    if gpe then return end
    if i.KeyCode==Enum.KeyCode.RightShift   then toggleESP()    end
    if i.KeyCode==Enum.KeyCode.RightControl then toggleHitbox() end
end)

-- ============================================================
-- DONE
-- ============================================================
game:GetService("StarterGui"):SetCore("SendNotification",{
    Title="Silent Assassins ESP",
    Text="✅ Loaded!\n🟢 Bình thường  🔴 Bị lộ\nRShift=ESP | RCtrl=Hitbox\nClick tên ESP để chọn kill target",
    Duration=4
})
print("✅ Silent Assassins ESP + Hitbox (Clean) loaded!")
