
import Foundation

final class ViewDataStore: ObservableObject {
    @Published var running: Bool = false
    
    @Published var enableTurboBoost: Bool = false
    @Published var enableProcHot: Bool = false
    @Published var hwpRequestValue: String = ""
    @Published var turboRatioLimits: [TRLItem] = []
    
    @Published var showErrorForSettings: Bool = false
    @Published var errorMsgForSettings: String = ""
    
    var settingsCancel: () -> () = {}
    var settingsConfirm: () -> () = {}
    var settingsApply: () -> () = {}
}

struct TRLItem: Identifiable {
    let id: Int
    var value: String
}

class ErrorWithMessage: LocalizedError {
    private var message: String = ""
    
    init(_ msg: String) {
        self.message = msg
    }
    
    public var localizedDescription: String {
        get {
            return self.message
        }
    }
}

func getPhysicalCPUCount() -> Int {
    var resultSize = 0
    sysctlbyname("hw.physicalcpu", nil, &resultSize, nil, 0)
    if resultSize <= 0 {
        return 1
    }
    var resultCString = [CChar](repeating: 0, count: resultSize)
    sysctlbyname("hw.physicalcpu", &resultCString, &resultSize, nil, 0)
    return Int(resultCString[0])
}

func subString(_ inputString: String, _ start: Int, _ end: Int) -> String {
    let s = start < 0 ? 0 : (start > inputString.count ? inputString.count : start)
    let e = end > inputString.count ? inputString.count : (end < 0 ? 0 : end)
    return String(inputString[
        inputString.index(inputString.startIndex, offsetBy: s) ..<
        inputString.index(inputString.startIndex, offsetBy: e)
    ])
}

func subString(_ inputString: String, _ start: Int) -> String {
    return subString(inputString, start, inputString.count)
}

func mapSettingsEntityToDataStore(_ settings: SettingsEntity, _ dataStore: ViewDataStore, _ cpuCount: Int) {
    dataStore.enableTurboBoost = settings.turboBoost
    dataStore.hwpRequestValue = settings.hwpRequest
    dataStore.enableProcHot = settings.procHot
    
    let trlValues = settings.getTurboRatioLimitAsArray()
    
    var trlList = Array(
        repeating: TRLItem(id: 0, value: ""),
        count: cpuCount
    )
    
    for i in 0..<cpuCount {
        if trlValues.count > i {
            trlList[i] = TRLItem(id: i, value: String(trlValues[i]))
        } else {
            trlList[i] = TRLItem(id: i, value: "")
        }
    }
    
    dataStore.turboRatioLimits = trlList
}

func mapDataStoreToSettingsEntity(_ dataStore: ViewDataStore, _ settings: SettingsEntity) {
    settings.turboBoost = dataStore.enableTurboBoost
    settings.hwpRequest = dataStore.hwpRequestValue
    settings.procHot = dataStore.enableProcHot
    
    let trlValues = dataStore.turboRatioLimits.map {v in
        UInt(v.value) ?? 0
    }
    settings.setTurboRatioLimitByArray(trlValues)
}
