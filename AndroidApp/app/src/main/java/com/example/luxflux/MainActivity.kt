package com.example.luxflux

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.os.Handler
import android.os.Message
import android.util.Log
import android.widget.Toast
import java.lang.ref.WeakReference
import java.net.NetworkInterface.getNetworkInterfaces

class MainActivity : AppCompatActivity() {
    private var handler: Handler

    init {
        val outerClass = WeakReference(this)
        handler = MyHandler(outerClass)
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

       // tv_ip.text = "HOST: ${getIPAddress()}"
       // tv_recieve.isSingleLine = false

        val serverThread = ServerThread()
        serverThread.start()
        serverThread.setHandler(handler)

    }

    //取得裝置IP
    private fun getIPAddress(): String {
        val IFCONFIG = StringBuilder()
        val en = getNetworkInterfaces()
        while (en.hasMoreElements()) {
            val intf = en.nextElement()
            val enumIpAddr = intf.inetAddresses
            while (enumIpAddr.hasMoreElements()) {
                val inetAddress = enumIpAddr.nextElement()
                if (!inetAddress.isLoopbackAddress && !inetAddress.isLinkLocalAddress && inetAddress.isSiteLocalAddress) {
                    IFCONFIG.append(inetAddress.hostAddress.toString() + "\n")
                }
            }
        }
        Log.d("TAG", IFCONFIG.toString())
        return IFCONFIG.toString()
    }


    // Declare the Handler as a static class.
    class MyHandler(private val outerClass: WeakReference<MainActivity>) : Handler() {
        /*
        override fun handleMessage(msg: Message?) {
            Log.d("Recieve", msg.toString())
           // outerClass.get()?.tv_recieve?.append(msg?.obj.toString()+"\n")
        }*/
    }

}