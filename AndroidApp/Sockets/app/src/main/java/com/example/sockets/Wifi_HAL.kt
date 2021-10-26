package com.example.sockets

import android.content.Context
import android.net.ConnectivityManager
import android.net.Network
import android.net.NetworkCapabilities
import android.net.NetworkCapabilities.TRANSPORT_CELLULAR
import android.net.NetworkCapabilities.TRANSPORT_WIFI
import android.net.NetworkRequest
import android.os.Build

sealed class NetworkState {
    data class Available(val type: NetworkType) : NetworkState()
    object Unavailable : NetworkState()
    object Connecting : NetworkState()
    object Losing : NetworkState()
    object Lost : NetworkState()
}

sealed class NetworkType {
    object WiFi : NetworkType()
    object CELL : NetworkType()
    object OTHER : NetworkType()
}

class NetworkReachabilityService private constructor(context: Context) {

    private val connectivityManager: ConnectivityManager =
        context.getSystemService(Context.CONNECTIVITY_SERVICE) as ConnectivityManager
    private val networkCallback = object : ConnectivityManager.NetworkCallback() {
        // There are more functions to override!

        override fun onLost(network: Network) {
            super.onLost(network)
            networkState = NetworkState.Lost
        }

        override fun onUnavailable() {
            super.onUnavailable()
            networkState = NetworkState.Unavailable
        }

        override fun onLosing(network: Network, maxMsToLive: Int) {
            super.onLosing(network, maxMsToLive)
            networkState = NetworkState.Losing
        }

        override fun onAvailable(network: Network) {
            super.onAvailable(network)
            updateAvailability(connectivityManager.getNetworkCapabilities(network))
        }

        override fun onCapabilitiesChanged(
            network: Network,
            networkCapabilities: NetworkCapabilities
        ) {
            super.onCapabilitiesChanged(network, networkCapabilities)
            updateAvailability(networkCapabilities)
        }
    }

    var networkState: NetworkState = NetworkState.Unavailable
        private set

    private fun updateAvailability(networkCapabilities: NetworkCapabilities?) {
        if (networkCapabilities == null) {
            networkState = NetworkState.Unavailable
            return
        }
        var networkType: NetworkType = NetworkType.OTHER

        if (networkCapabilities.hasTransport(TRANSPORT_CELLULAR)) {
            networkType = NetworkType.CELL
        }
        if (networkCapabilities.hasTransport(TRANSPORT_WIFI)) {
            networkType = NetworkType.WiFi
        }

        networkState = NetworkState.Available(networkType)
    }

    fun pauseListeningNetworkChanges() {
        try {
            connectivityManager.unregisterNetworkCallback(networkCallback)
        } catch (e: IllegalArgumentException) {
            // Usually happens only once if: "NetworkCallback was not registered"
        }
    }

    fun resumeListeningNetworkChanges() {
        pauseListeningNetworkChanges()
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
            connectivityManager.registerDefaultNetworkCallback(networkCallback)
        } else {
            connectivityManager.registerNetworkCallback(
                NetworkRequest.Builder().build(),
                networkCallback
            )
        }
    }
}