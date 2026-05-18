$JLinkConfig = @{
    Device = "STM32F407ZE"
    Interface = "SWD"
    Speed = "2000"
    Port = 2331
    TimeoutSeconds = 10
}

function Resolve-JLinkStringValue($Value, $Name) {
    if ($JLinkConfig.ContainsKey($Name) -and ![string]::IsNullOrWhiteSpace([string]$JLinkConfig[$Name])) {
        return [string]$JLinkConfig[$Name]
    }

    if (![string]::IsNullOrWhiteSpace($Value)) {
        return $Value
    }

    throw "Missing J-Link config value: $Name"
}

function Resolve-JLinkIntValue($Value, $Name) {
    if ($JLinkConfig.ContainsKey($Name) -and ($null -ne $JLinkConfig[$Name])) {
        return [int]$JLinkConfig[$Name]
    }

    if (($null -ne $Value) -and ($Value -ne 0)) {
        return [int]$Value
    }

    throw "Missing J-Link config value: $Name"
}
