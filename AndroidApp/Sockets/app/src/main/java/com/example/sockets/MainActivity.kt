package com.example.sockets

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import io.ktor.network.selector.*
import io.ktor.network.sockets.*
import io.ktor.utils.io.*
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.GlobalScope
import kotlinx.coroutines.launch
import kotlinx.coroutines.runBlocking
import java.net.InetSocketAddress




class MainActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        GlobalScope.launch(Dispatchers.IO) {
            val server = aSocket(ActorSelectorManager(Dispatchers.IO)).tcp().bind(InetSocketAddress("0.0.0.0", 3333))
            //println("Started echo telnet server at ${server.localAddress}")

            while (true) {
                val socket = server.accept()

                launch(Dispatchers.Main) {
                    //println("Socket accepted: ${socket.remoteAddress}")

                    val input = socket.openReadChannel()
                    val output = socket.openWriteChannel(autoFlush = true)

                    try {
                        while (true) {
                            val line = input.readUTF8Line(Int.MAX_VALUE)
                            //println("${socket.remoteAddress}: $line")
                            output.writeStringUtf8("$line\r\n")
                        }
                    } catch (e: Throwable) {
                        e.printStackTrace()
                        socket.close()
                    }
                }
            }
        }
    }
}