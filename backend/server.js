import express from 'express';
import cors from 'cors';
const app=express();app.use(cors());app.use(express.json());
app.get('/health',(_req,res)=>res.json({success:true,status:'ok'}));
// Demo-only endpoints so the firmware can be tested on a LAN before merging into your real backend.
// They do not persist sessions, calculate actual parking fees, or authenticate devices.
app.post('/api/entry/manual-entry',(req,res)=>{const plateNumber=String(req.body?.plateNumber||'').toUpperCase().replace(/[^A-Z0-9]/g,'');if(plateNumber.length<4)return res.status(400).json({success:false,error:'Valid plate number required'});res.status(201).json({success:true,data:{plateNumber,slotId:'L1-A05',level:'L1',displayMessage:'Welcome! Proceed to L1-A05'}})});
app.post('/api/exit/manual-scan',(req,res)=>{const plateNumber=String(req.body?.plateNumber||'').toUpperCase().replace(/[^A-Z0-9]/g,'');if(plateNumber.length<4)return res.status(400).json({success:false,error:'Plate number required'});res.json({success:true,data:{plateNumber,feeBreakdown:{totalAmount:35},payment:{amount:35},message:'DEMO ONLY: payment not verified'}})});
const port=5000;app.listen(port,'0.0.0.0',()=>console.log(`SmartPark demo API listening on http://0.0.0.0:${port}`));
