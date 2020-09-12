
import SwiftUI

struct SettingsView: View {
    @EnvironmentObject var dataStore: ViewDataStore
    
    var body: some View {
        VStack {
            VStack(alignment: .leading) {
                Toggle(isOn: self.$dataStore.enableTurboBoost) {
                    Text(LocalizedStringKey("Enable turbo boost"))
                }
                Toggle(isOn: self.$dataStore.enableProcHot) {
                    Text(LocalizedStringKey("Enable proc hot"))
                }
                HStack {
                    Text(LocalizedStringKey("HWP request value"))
                        .multilineTextAlignment(.leading)
                        .frame(minWidth:110, alignment: .leading)
                    TextField(
                        LocalizedStringKey("HWP request value"),
                        text: self.$dataStore.hwpRequestValue
                    )
                }
                HStack {
                    Text(LocalizedStringKey("Turbo ratio limit"))
                        .frame(minWidth:100, alignment: .leading)
                    ForEach(self.dataStore.turboRatioLimits.indices) { i in
                        TextField(
                            LocalizedStringKey("core\(i + 1)"),
                            text: self.$dataStore.turboRatioLimits[i].value
                        )
                    }
                }
            }.padding()
            HStack {
                Button(action: {self.dataStore.settingsCancel()}) {
                    Text(LocalizedStringKey("Cancel"))
                }
                Spacer()
                Button(action: {self.dataStore.settingsConfirm()}) {
                    Text(LocalizedStringKey("Confirm"))
                }
                Button(action: {self.dataStore.settingsApply()}) {
                    Text(LocalizedStringKey("Apply"))
                }
            }.padding()
        }.padding().frame(minWidth: 550).alert(isPresented: self.$dataStore.showErrorForSettings) {
            Alert(
                title: Text("Error"),
                message: Text(self.dataStore.errorMsgForSettings),
                dismissButton: .default(Text("Confirm"))
            )
        }
    }
}


private func createDataStoreForPreview() -> ViewDataStore {
    let dataStore = ViewDataStore()
    dataStore.turboRatioLimits = [
        TRLItem(id: 0, value: ""),
        TRLItem(id: 1, value: ""),
        TRLItem(id: 2, value: ""),
        TRLItem(id: 3, value: ""),
        TRLItem(id: 4, value: ""),
        TRLItem(id: 5, value: ""),
    ]
    return dataStore
}

struct SettingsView_Previews: PreviewProvider {
    static var previews: some View {
        SettingsView().environmentObject(createDataStoreForPreview())
    }
}
