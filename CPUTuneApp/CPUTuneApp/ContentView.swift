//
//  ContentView.swift
//  CPUTuneApp
//
//  Created by Yating Zhou on 7/5/20.
//  Copyright © 2020 syscl. All rights reserved.
//

import SwiftUI

struct ContentView: View {
    @EnvironmentObject var dataStore: ViewDataStore
    
    var body: some View {
        VStack {
            Text("Hello, World!")
                .frame(maxWidth: .infinity, maxHeight: .infinity)
            HStack {
                Button(action: {}) {Text("Run")}
            }
        }.padding()
    }
}

struct ContentView_Previews: PreviewProvider {
    static var previews: some View {
        ContentView()
    }
}
