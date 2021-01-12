VERSION 5.00
Object = "{831FDD16-0C5C-11D2-A9FC-0000F8754DA1}#2.2#0"; "MSCOMCTL.OCX"
Begin VB.Form formCheats 
   Caption         =   "Apply Cheats"
   ClientHeight    =   8310
   ClientLeft      =   120
   ClientTop       =   450
   ClientWidth     =   10080
   BeginProperty Font 
      Name            =   "Lucida Console"
      Size            =   9.75
      Charset         =   0
      Weight          =   400
      Underline       =   0   'False
      Italic          =   0   'False
      Strikethrough   =   0   'False
   EndProperty
   Icon            =   "formApplyCheats.frx":0000
   KeyPreview      =   -1  'True
   LockControls    =   -1  'True
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   8310
   ScaleWidth      =   10080
   StartUpPosition =   1  'CenterOwner
   Begin VB.CommandButton btnCollapse 
      BackColor       =   &H00FFFFFF&
      Caption         =   "-"
      BeginProperty Font 
         Name            =   "Lucida Console"
         Size            =   9.75
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   300
      Left            =   2310
      Style           =   1  'Graphical
      TabIndex        =   9
      TabStop         =   0   'False
      Top             =   255
      Width           =   330
   End
   Begin VB.CommandButton btnExpand 
      BackColor       =   &H00FFFFFF&
      Caption         =   "+"
      BeginProperty Font 
         Name            =   "Lucida Console"
         Size            =   9.75
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   300
      Left            =   1995
      Style           =   1  'Graphical
      TabIndex        =   10
      TabStop         =   0   'False
      Top             =   255
      Width           =   330
   End
   Begin VB.CommandButton btnBack 
      BackColor       =   &H00FFFFFF&
      BeginProperty Font 
         Name            =   "Verdana"
         Size            =   9
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   555
      Left            =   705
      Picture         =   "formApplyCheats.frx":1242
      Style           =   1  'Graphical
      TabIndex        =   6
      TabStop         =   0   'False
      ToolTipText     =   "Preview Cheats"
      Top             =   7725
      Width           =   690
   End
   Begin MSComctlLib.TreeView tvCheats 
      Height          =   6660
      Left            =   15
      TabIndex        =   0
      Top             =   1035
      Width           =   10065
      _ExtentX        =   17754
      _ExtentY        =   11748
      _Version        =   393217
      HideSelection   =   0   'False
      Indentation     =   353
      LabelEdit       =   1
      Style           =   7
      Checkboxes      =   -1  'True
      Appearance      =   1
   End
   Begin VB.CommandButton btnEdit 
      BackColor       =   &H00FFFFFF&
      BeginProperty Font 
         Name            =   "Verdana"
         Size            =   9
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   555
      Left            =   15
      Picture         =   "formApplyCheats.frx":1E84
      Style           =   1  'Graphical
      TabIndex        =   3
      TabStop         =   0   'False
      ToolTipText     =   "Edit patch file  (Ctrl+E)"
      Top             =   7725
      Width           =   690
   End
   Begin VB.CommandButton btnDownload 
      BackColor       =   &H00FFFFFF&
      BeginProperty Font 
         Name            =   "Verdana"
         Size            =   8.25
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   555
      Left            =   15
      MaskColor       =   &H00140B05&
      Picture         =   "formApplyCheats.frx":2A06
      Style           =   1  'Graphical
      TabIndex        =   5
      TabStop         =   0   'False
      ToolTipText     =   "Download patch file from PS3Cheating.net Repository (Ctrl+D)"
      Top             =   6000
      Visible         =   0   'False
      Width           =   660
   End
   Begin VB.TextBox txtCheats 
      ForeColor       =   &H00FF0000&
      Height          =   4950
      HideSelection   =   0   'False
      Left            =   0
      MultiLine       =   -1  'True
      ScrollBars      =   3  'Both
      TabIndex        =   4
      Top             =   1035
      Visible         =   0   'False
      Width           =   6765
   End
   Begin VB.CommandButton btnClose 
      BackColor       =   &H00FFFFFF&
      Cancel          =   -1  'True
      Caption         =   "Close"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   8.25
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   555
      Left            =   8445
      Style           =   1  'Graphical
      TabIndex        =   2
      Top             =   7725
      Width           =   1620
   End
   Begin VB.CommandButton btnApply 
      BackColor       =   &H00FFFFFF&
      Caption         =   "Apply >"
      Default         =   -1  'True
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   8.25
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   555
      Left            =   6795
      Style           =   1  'Graphical
      TabIndex        =   1
      Top             =   7725
      Width           =   1620
   End
   Begin VB.Label lblStats 
      BeginProperty Font 
         Name            =   "Verdana"
         Size            =   9.75
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H00FF0000&
      Height          =   510
      Left            =   1665
      TabIndex        =   8
      Top             =   7875
      Width           =   4740
   End
   Begin VB.Label lblTitle 
      BackColor       =   &H00FFFFFF&
      BeginProperty Font 
         Name            =   "Tahoma"
         Size            =   11.25
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H00FF0000&
      Height          =   330
      Left            =   2010
      TabIndex        =   7
      Top             =   660
      UseMnemonic     =   0   'False
      Width           =   7410
   End
   Begin VB.Image imgLogo 
      Height          =   1020
      Left            =   120
      MousePointer    =   99  'Custom
      Stretch         =   -1  'True
      Top             =   15
      Width           =   1755
   End
   Begin VB.Shape shpLogo 
      BackColor       =   &H00FFFFFF&
      BorderColor     =   &H8000000A&
      FillColor       =   &H00FFFFFF&
      FillStyle       =   0  'Solid
      Height          =   1050
      Left            =   0
      Top             =   0
      Width           =   13785
   End
End
Attribute VB_Name = "formCheats"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit
  
Private Declare Function URLDownloadToFile Lib "urlmon" Alias "URLDownloadToFileA" (ByVal pCaller As Long, ByVal szURL As String, ByVal szFileName As String, ByVal dwReserved As Long, ByVal lpfnCB As Long) As Long
  
Private ReplaceStart As Long
Private ReplaceCount As Long
Private ReplaceMode As Boolean
Private ReplaceText As String
Private FindText As String
Private DataChanged As Boolean

Public TitleID As String
Public savepath As String
Public PatchFile As String

Private crc As New clsCRC32
Private md5 As New clsMD5

Const Ox As String = "&H"
Const VariableMask As String = "[[]*[]]"
Private Const OutDir As String = "~extracted"

Private dos As New DOSOutputs
Private Const SavePatchExtension As String = ".ps3savepatch"

Private TotalCheats As Long

Private Sub btnBack_Click()
  Dim b As Boolean
  On Local Error Resume Next
  If txtCheats.Visible Then
    If DataChanged = False Then
      DataChanged = txtCheats.DataChanged
    End If
    SavePatch PatchFile
    txtCheats.Visible = False
    btnEdit.Visible = True
    btnBack.Visible = False
    tvCheats.Visible = True
    ReadCheats PatchFile
    txtCheats.DataChanged = DataChanged
    Me.Caption = "Edit Cheats [" & ExtractFile(PatchFile) & IIf(txtCheats.DataChanged, "*", "") & "]"
    tvCheats.SetFocus
  End If
End Sub

Private Sub btnExpand_Click()
  On Local Error Resume Next
  Form_KeyDown 187, 0
  tvCheats.SetFocus
End Sub

Private Sub btnCollapse_Click()
  On Local Error Resume Next
  Form_KeyDown 189, 0
  tvCheats.SetFocus
End Sub

Private Sub Form_QueryUnload(Cancel As Integer, UnloadMode As Integer)
  On Local Error Resume Next
  If txtCheats.DataChanged Then
    Select Case MsgBox("Do you want save the changes before close the window?", vbQuestion + vbYesNoCancel)
     Case vbYes: SavePatch PatchFile
     Case vbCancel: Cancel = -1
     Case vbNo
       If FileExists(PatchFile & ".tmp") Then
         FileCopy PatchFile & ".tmp", PatchFile
       End If
    End Select
  End If
End Sub

Private Sub Form_Unload(Cancel As Integer)
  On Local Error Resume Next
  SaveSetting App.title, "Cheats", "Width", Me.Width
  SaveSetting App.title, "Cheats", "Height", Me.Height
  Kill PatchFile & ".tmp"
  Set dos = Nothing
End Sub

Private Sub tvCheats_DblClick()
  Dim IsURL As Boolean, pos As Long, url As String
  On Local Error Resume Next
  IsURL = IsIn(tvCheats.SelectedItem.Text, "*http://*", "*https://*", "*ftp://*", "*file://*", "*mailto://*")
  
  If IsURL Then
    pos = InStr(tvCheats.SelectedItem.Text, "http://")
    If pos = 0 Then
      pos = InStr(tvCheats.SelectedItem.Text, "https://")
    End If
    If pos = 0 Then
      pos = InStr(tvCheats.SelectedItem.Text, "ftp://")
    End If
    If pos = 0 Then
      pos = InStr(tvCheats.SelectedItem.Text, "file://")
    End If
    If pos = 0 Then
      pos = InStr(tvCheats.SelectedItem.Text, "mailto://")
    End If
    If pos Then
      url = Mid$(tvCheats.SelectedItem.Text, pos)
      pos = InStr(url, " ")
      If pos Then
        url = Left$(tvCheats.SelectedItem.Text, pos - 1)
      End If

      ShellExecute 0, vbNullString, url, vbNullString, vbNullString, vbNormalFocus
    End If
  Else
    tvCheats.SelectedItem.Checked = Not tvCheats.SelectedItem.Checked
  End If
  RefreshStats
End Sub

Private Sub tvCheats_KeyUp(KeyCode As Integer, Shift As Integer)
  On Local Error Resume Next
  If KeyCode = vbKeySpace Then
    RefreshStats
  End If
End Sub

Private Sub tvCheats_MouseUp(Button As Integer, Shift As Integer, X As Single, Y As Single)
  Dim node As node
  On Local Error Resume Next
  Set node = tvCheats.HitTest(X, Y)
  If node Is Nothing Then
    'skip
  ElseIf GetAsyncKeyState(vbKeyControl) < 0 Then
    'skip
  ElseIf LCase$(node.Text) Like "info:*" Then
    node.Checked = False
    tvCheats_NodeCheck node
  ElseIf LCase$(node.Text) Like "*(required)*" Then
    node.Checked = True
    tvCheats_NodeCheck node
  End If
End Sub

Private Sub tvCheats_NodeCheck(ByVal node As MSComctlLib.node)
  Dim i As Integer, ParentKey As String, ParentText As String, b As Boolean
  On Local Error Resume Next
  
  ParentKey = node.Parent.Key
  ParentText = node.Parent.Text

  If InStr(1, node.Text, "(choose one)", vbTextCompare) Or InStr(1, node.Text, "(select one)", vbTextCompare) Then
    'skip
  ElseIf InStr(1, ParentText, "(choose one)", vbTextCompare) Or InStr(1, ParentText, "(select one)", vbTextCompare) Then
    LockWindowUpdate tvCheats.hWnd
    For i = node.Parent.Index + 1 To tvCheats.Nodes.Count
      If Len(tvCheats.Nodes(i).Parent.Key) = 0 Then
        'skip
      ElseIf tvCheats.Nodes(i).Parent.Key = ParentKey Then
        tvCheats.Nodes(i).Checked = False
      Else
        Exit For
      End If
    Next
    node.Checked = b
    LockWindowUpdate 0&
  ElseIf Len(node.Key) Then
    LockWindowUpdate tvCheats.hWnd
    For i = node.Index + 1 To tvCheats.Nodes.Count
      If Len(tvCheats.Nodes(i).Parent.Key) = 0 Then
        'skip
      ElseIf tvCheats.Nodes(i).Parent.Key = node.Key Then
        tvCheats.Nodes(i).Checked = node.Checked
      Else
        Exit For
      End If
    Next
    LockWindowUpdate 0&
  End If

  RefreshStats
End Sub

Private Sub RefreshStats()
  Dim TotalSelected As Long, i As Long, Text As String
  On Local Error Resume Next
  With tvCheats.Nodes
    TotalSelected = 0
    For i = 1 To .Count
      Text = LCase$(.item(i))
      If .item(i).Checked Then
        If Text Like "info:*" Then
          .item(i).Checked = False
        ElseIf .item(i).Children = 0 Then
          TotalSelected = TotalSelected + 1
        End If
      ElseIf Text Like "*(required)" Then
        .item(i).Checked = True
      End If
    Next
  End With
  lblStats.Caption = TotalSelected & " Cheats Selected  ·  " & TotalCheats & " Total Cheats"
End Sub

Private Sub txtCheats_Change()
  On Local Error Resume Next
  txtCheats.DataChanged = True
  Me.Caption = "Edit Cheats [" & ExtractFile(PatchFile) & "*]"
End Sub

Public Sub btnEdit_Click()
  On Local Error Resume Next
  If FileExists(PatchFile & ".tmp") = False Then
    FileCopy PatchFile, PatchFile & ".tmp"
  End If
  
  txtCheats.Text = FixLineBreaks(PatchFile)
  txtCheats.DataChanged = DataChanged
  Me.Caption = "Edit Cheats [" & ExtractFile(PatchFile) & IIf(txtCheats.DataChanged, "*", "") & "]"

  txtCheats.Visible = True
  tvCheats.Visible = False
  btnEdit.Visible = False
  btnBack.Visible = True
  btnApply.Caption = "&Save >"
  btnApply.Enabled = True
  btnApply.default = False
  txtCheats.SetFocus
End Sub

Private Sub imgLogo_Click()
  On Local Error Resume Next
  ShellExecute 0, vbNullString, imgLogo.ToolTipText, vbNullString, vbNullString, vbNormalFocus
End Sub

Private Sub Form_Load()
  On Local Error Resume Next
  
  Me.Width = GetSetting(App.title, "Cheats", "Width", Me.Width)
  Me.Height = GetSetting(App.title, "Cheats", "Height", Me.Height)

  If formMain.imgCover.Visible Then
    Set imgLogo.Picture = formMain.imgCover.Picture
    imgLogo.ToolTipText = formMain.imgCover.ToolTipText
  End If

  Kill PatchFile & ".tmp"
  ReadCheats PatchFile
  txtCheats.DataChanged = False
  DataChanged = False
  Me.Caption = "Apply Cheats [" & ExtractFile(PatchFile) & "]"
  imgLogo.MouseIcon = formMain.lblCredits.MouseIcon
End Sub

Private Sub Form_Resize()
  On Local Error Resume Next
  shpLogo.Width = ScaleWidth * 2
  btnEdit.Top = ScaleHeight - btnEdit.Height - Screen.TwipsPerPixelY * 2
  btnBack.Move btnEdit.Left, btnEdit.Top
  btnDownload.Top = btnEdit.Top
  btnClose.Move ScaleWidth - btnClose.Width - Screen.TwipsPerPixelY * 2, btnEdit.Top
  btnApply.Move btnClose.Left - btnApply.Width - Screen.TwipsPerPixelY * 2, btnEdit.Top
  btnEdit.Visible = (btnApply.Left > btnEdit.Left + btnEdit.Width) And tvCheats.Visible
  btnBack.Visible = (btnApply.Left > btnEdit.Left + btnEdit.Width) And Not tvCheats.Visible
  btnDownload.Visible = (btnApply.Left > btnDownload.Left + btnDownload.Width) 'And tvCheats.Visible
  lblTitle.Width = ScaleWidth - lblTitle.Left

  tvCheats.Move tvCheats.Left, tvCheats.Top, ScaleWidth - 2 * tvCheats.Left, btnEdit.Top - tvCheats.Top - Screen.TwipsPerPixelY
  txtCheats.Move tvCheats.Left, tvCheats.Top, ScaleWidth - 2 * tvCheats.Left, btnEdit.Top - tvCheats.Top - Screen.TwipsPerPixelY

  lblStats.Move lblStats.Left, btnEdit.Top + Screen.TwipsPerPixelY * 4, btnApply.Left - lblStats.Left

  If txtCheats.Visible Then txtCheats.SetFocus
End Sub

Private Sub Form_KeyDown(KeyCode As Integer, Shift As Integer)
  Dim i As Long
  On Local Error Resume Next
  If KeyCode = vbKeyE And (Shift = vbCtrlMask Or Shift = vbAltMask) Then
    If txtCheats.Visible = False Then
      btnEdit_Click
      KeyCode = 0
    End If
  'ElseIf KeyCode = vbKeyD And (Shift = vbCtrlMask Or Shift = vbAltMask) Then
  '  btnDownload_Click
  '  KeyCode = 0
  ElseIf KeyCode = vbKeyG And (Shift = vbCtrlMask Or Shift = vbAltMask) Then
    btnBack_Click
    KeyCode = 0
  ElseIf KeyCode = vbKeyA And Shift = vbCtrlMask Then
    If txtCheats.Visible Then
      txtCheats.selstart = 0
      txtCheats.SelLength = Len(txtCheats.Text)
    Else
      LockWindowUpdate tvCheats.hWnd
      With tvCheats.Nodes
        For i = 1 To tvCheats.Nodes.Count
          If LCase$(.item(i).Text) Like "info:*" Then
            .item(i).Checked = False
          Else
            .item(i).Checked = True
          End If
        Next
      End With
      tvCheats.SelectedItem.EnsureVisible
      RefreshStats
      LockWindowUpdate 0&
    End If
    KeyCode = 0
  ElseIf KeyCode = vbKeyN And Shift = vbCtrlMask Then
    LockWindowUpdate tvCheats.hWnd
    With tvCheats.Nodes
      For i = 1 To tvCheats.Nodes.Count
        .item(i).Checked = .item(i).Tag
      Next
    End With
    tvCheats.SelectedItem.EnsureVisible
    RefreshStats
    LockWindowUpdate 0&
    KeyCode = 0
  ElseIf KeyCode = vbKeyI And Shift = vbCtrlMask Then
    LockWindowUpdate tvCheats.hWnd
    With tvCheats.Nodes
      For i = 1 To tvCheats.Nodes.Count
        If .item(i).Tag Then
          .item(i).Checked = True
        Else
          .item(i).Checked = Not .item(i).Checked
        End If
      Next
    End With
    tvCheats.SelectedItem.EnsureVisible
    RefreshStats
    LockWindowUpdate 0&
    KeyCode = 0
  ElseIf KeyCode = 187 Or KeyCode = vbKeyAdd Then
    LockWindowUpdate tvCheats.hWnd
    With tvCheats.Nodes
      For i = 1 To tvCheats.Nodes.Count
        .item(i).Expanded = True
      Next
    End With
    tvCheats.SelectedItem.EnsureVisible
    LockWindowUpdate 0&
    KeyCode = 0
  ElseIf KeyCode = 189 Or KeyCode = vbKeySubtract Then
    LockWindowUpdate tvCheats.hWnd
    With tvCheats.Nodes
      For i = 1 To tvCheats.Nodes.Count
        .item(i).Expanded = False
      Next
    End With
    tvCheats.SelectedItem.EnsureVisible
    LockWindowUpdate 0&
    KeyCode = 0
  ElseIf KeyCode = vbKeyF And Shift = vbCtrlMask Then
    ReplaceMode = False
    Find Ask:=True
    KeyCode = 0
  ElseIf KeyCode = vbKeyR And Shift = vbCtrlMask Then
    ReplaceMode = True
    Find Ask:=True
    KeyCode = 0
  ElseIf KeyCode = vbKeyF3 Then
    Find Ask:=False
    KeyCode = 0
  ElseIf KeyCode = vbKeyS And Shift = vbCtrlMask Then
    SavePatch PatchFile
    DataChanged = False
    Me.Caption = "Edit Cheats [" & ExtractFile(PatchFile) & "]"
    KeyCode = 0
  End If
End Sub

Private Sub Find(Optional Ask As Boolean)
  Dim Text As String, pos As Long
  On Local Error Resume Next
  If Ask Or Len(FindText) = 0 Then
    Text = InputBox("Find what:", "Find", FindText)
    If Len(Text) = 0 Then Exit Sub
    FindText = Text
    If ReplaceMode And txtCheats.Visible Then
      Text = InputBox("Replace '" & FindText & "' with:", "Replace", ReplaceText)
      If StrPtr(Text) = 0 Then Exit Sub
      ReplaceText = Text
      ReplaceCount = 0
      ReplaceStart = txtCheats.selstart
    End If
  End If
  
  If txtCheats.Visible Then
    pos = InStr(txtCheats.selstart + 2, txtCheats.Text, FindText, vbTextCompare)
    If pos Then
      txtCheats.selstart = pos - 1
      txtCheats.SelLength = Len(FindText)
      If ReplaceMode Then
        If GetAsyncKeyState(vbKeyControl) < 0 And Ask = False Then
          pos = vbYes
        Else
          pos = MsgBox("Do you want to replace '" & FindText & "' with '" & ReplaceText & "'?", vbQuestion + vbYesNoCancel)
        End If
        Select Case pos
         Case vbNo: Find Ask:=False
         Case vbYes: ReplaceCount = ReplaceCount + 1
                     txtCheats.SelText = ReplaceText: Find Ask:=False
                     If Ask Then MsgBox ReplaceCount & " occourences have been replaced", vbInformation
         Case Else
        End Select
      End If
    Else
      If ReplaceMode Then
        If ReplaceStart = 0 Then
          Exit Sub
        ElseIf MsgBox("Do you want to continue replacing from the begining?", vbYesNoCancel) <> vbYes Then
          Exit Sub
        End If
      End If

      pos = InStr(1, txtCheats.Text, FindText, vbTextCompare)
      If pos Then
        txtCheats.selstart = pos - 1
        txtCheats.SelLength = Len(FindText)
        If ReplaceMode Then
          If GetAsyncKeyState(vbKeyControl) < 0 And Ask = False Then
            pos = vbYes
          Else
            pos = MsgBox("Do you want to replace '" & FindText & "' with '" & ReplaceText & "'?", vbQuestion + vbYesNoCancel)
          End If
          Select Case pos
           Case vbNo: Find Ask:=False
           Case vbYes: ReplaceCount = ReplaceCount + 1
                       txtCheats.SelText = ReplaceText: Find Ask:=False
                       If Ask Then MsgBox ReplaceCount & " occourences have been replaced", vbInformation
           Case Else
          End Select
        End If
      ElseIf Ask Then
        MsgBox "'" & FindText & "' was not found."
      End If
    End If
  Else
    If InStr(FindText, "*") = 0 Then
      Text = "*" & LCase$(FindText) & "*"
    Else
      Text = LCase$(FindText)
    End If

    For pos = tvCheats.SelectedItem.Index + 1 To tvCheats.Nodes.Count
      If LCase$(tvCheats.Nodes(pos).Text) Like LCase$(Text) Then
        Set tvCheats.SelectedItem = tvCheats.Nodes(pos)
        tvCheats.SelectedItem.EnsureVisible
        Exit Sub
      End If
    Next
  
    For pos = 1 To tvCheats.SelectedItem.Index
      If LCase$(tvCheats.Nodes(pos).Text) Like LCase$(Text) Then
        Set tvCheats.SelectedItem = tvCheats.Nodes(pos)
        tvCheats.SelectedItem.EnsureVisible
        Exit Sub
      End If
    Next

    MsgBox "'" & FindText & "' was not found."
  End If
End Sub

Private Sub btnApply_Click()
  On Local Error Resume Next
  If btnApply.Caption = "&Save >" Then
    SavePatch PatchFile
    DataChanged = False
    txtCheats.DataChanged = DataChanged
  Else
    ApplyPatch PatchFile, savepath
    formMain.CheatApplied = TitleID
  End If
  Unload Me
End Sub

Private Sub SavePatch(PatchFile As String)
  Dim fh As Long, Text As String
  On Local Error Resume Next
  If txtCheats.DataChanged Then
    FileCopy PatchFile, PatchFile & ".bak"
    Kill PatchFile
    Text = txtCheats.Text
    fh = FreeFile
    Open PatchFile For Binary As #fh
    Put #fh, 1, Text
    Close #fh
    txtCheats.DataChanged = False
  End If
End Sub

Private Sub btnClose_Click()
  On Local Error Resume Next
  Unload Me
End Sub

Public Sub ReadCheats(ByVal PatchFile As String)
  Dim Data As Variant, i As Long, pos As Long
  Dim line As String, fh As Long, Group As String, nodex As node
  On Local Error Resume Next
  tvCheats.Nodes.Clear
  Me.Caption = "Apply Cheats [" & ExtractFile(PatchFile) & "]"

  If FileExists(PatchFile) Then
    LockWindowUpdate Me.hWnd
  
    FixLineBreaks PatchFile

    fh = FreeFile
    Open PatchFile For Input As #fh
    Do Until EOF(fh)
      line = ""
      Line Input #fh, line
      line = Trim$(line)

      '-- remove remarks
      pos = modVB6.InStrRev(line, ";")
      If pos Then
        If line Like "; --- * ---" Then
          Group = Mid$(line, 7, Len(line) - 10)
        ElseIf InStr(line, quote) Then
          line = Trim$(Left$(line, modVB6.InStrRev(line, quote)))
        Else
          line = Trim$(Left$(line, pos - 1))
        End If
      End If

      '--- General sections: :File / ; --- group --- / [cheat] / path: *
      If line Like "[0-9A-Fa-f]=*" Or line Like "[0-9A-Fa-f][0-9A-Fa-f]=*" Or line Like "[0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f]=*" Then
        tvCheats.Nodes.Remove nodex.Index
        Set nodex = Nothing
      '-- path: <required path pattern>
      ElseIf LCase$(line) Like "path:*" Then
        line = "\" & NormalizePath(Trim$(Mid$(line, 6)))
        If LCase$(Right$(NormalizePath(savepath), Len(line))) Like LCase$(line) Then Exit Do
      
      '-- group (alternative format)
      ElseIf line Like "; --- * ---" Then
        line = Mid$(line, 7, Len(line) - 10)
        Group = line
        If Group Like "*\*" Then
          Data = modVB6.Split(Group, "\")
          line = Data(UBound(Data))
          Set nodex = tvCheats.Nodes.Add(tvCheats.Nodes(Left$(Group, Len(Group) - Len(line) - 1)), tvwChild, Group, line)
        Else
          Set nodex = tvCheats.Nodes.Add(, , Group, Group)
        End If
        nodex.Bold = True
        nodex.Expanded = True
      
      '-- cheat label
      ElseIf line Like VariableMask Then
        line = RemoveBrackets(line)

        '-- group label
        If LCase$(line) Like "group:*" Then
          line = Trim$(Mid$(line, 7))
          Group = line
          If Group = "\" Then
            Group = ""
            Set nodex = Nothing
          ElseIf Group Like "*\*" Then
            Data = modVB6.Split(Group, "\")
            line = Data(UBound(Data))
            Set nodex = tvCheats.Nodes.Add(tvCheats.Nodes(Left$(Group, Len(Group) - Len(line) - 1)), tvwChild, Group, line)
          Else
            Set nodex = tvCheats.Nodes.Add(, , Group, Group)
          End If
          If nodex.Key = Group Then
            nodex.Bold = True
          End If
          nodex.Expanded = True
        ElseIf Len(Group) > 0 And Not line Like "*\*" Then
          If Len(tvCheats.Nodes(line)) = 0 Then
            Set nodex = tvCheats.Nodes.Add(Group, tvwChild, line, line)
          Else
            Set nodex = tvCheats.Nodes.Add(Group, tvwChild, , line)
          End If
          nodex.Expanded = True
        Else
          If line Like "*\*" Then
            Data = modVB6.Split(line, "\")
            Group = line
            line = Data(UBound(Data))
            Group = Left$(Group, Len(Group) - Len(line) - 1)
            If Len(Group) = 0 Then
              Set nodex = tvCheats.Nodes.Add(, , line, line)
              nodex.Expanded = True
            ElseIf Len(tvCheats.Nodes(Group)) = 0 Then
              For i = 1 To tvCheats.Nodes.Count
                If tvCheats.Nodes(i).Text = Group Then
                  Set nodex = tvCheats.Nodes.Add(tvCheats.Nodes(i), tvwChild, line, line)
                  nodex.Expanded = True
                  Exit For
                End If
              Next
            Else
              Set nodex = tvCheats.Nodes.Add(tvCheats.Nodes(Group), tvwChild, line, line)
              nodex.Expanded = True
            End If
          Else
            Set nodex = tvCheats.Nodes.Add(, , , line)
            nodex.Expanded = True
          End If
        End If

        '-- checkmark cheat on load
        If LCase$(nodex.Text) Like "default:*" Then
          nodex.Text = Trim$(Mid$(nodex.Text, 9))
          nodex.Checked = True
        ElseIf LCase$(nodex.Text) Like "*(required)" Then
          nodex.Checked = True
        End If

        '-- label background color
        If LCase$(nodex.Text) Like "yellow:*" Then
          nodex.Text = Trim$(Mid$(nodex.Text, 8))
          nodex.BackColor = vbYellow
        ElseIf LCase$(nodex.Text) Like "green:*" Then
          nodex.Text = Trim$(Mid$(nodex.Text, 7))
          nodex.BackColor = vbGreen
        ElseIf LCase$(nodex.Text) Like "blue:*" Then
          nodex.Text = Trim$(Mid$(nodex.Text, 6))
          nodex.BackColor = vbBlue
          nodex.ForeColor = vbWhite
        ElseIf LCase$(nodex.Text) Like "red:*" Then
          nodex.Text = Trim$(Mid$(nodex.Text, 5))
          nodex.BackColor = vbRed
          nodex.ForeColor = vbWhite
        ElseIf LCase$(nodex.Text) Like "orange:*" Then
          nodex.Text = Trim$(Mid$(nodex.Text, 8))
          nodex.BackColor = &H80FF&
        ElseIf LCase$(nodex.Text) Like "cyan:*" Then
          nodex.Text = Trim$(Mid$(nodex.Text, 6))
          nodex.BackColor = vbCyan
        ElseIf LCase$(nodex.Text) Like "magenta:*" Then
          nodex.Text = Trim$(Mid$(nodex.Text, 9))
          nodex.BackColor = vbMagenta
        ElseIf LCase$(nodex.Text) Like "gray:*" Then
          nodex.Text = Trim$(Mid$(nodex.Text, 6))
          nodex.BackColor = &HC0C0C0
        End If

        '-- label info / checkmarked
        If LCase$(nodex.Text) Like "info:*" Then
           nodex.Bold = True
        ElseIf LCase$(nodex.Text) Like "default:*" Then
           nodex.Text = Trim$(Mid$(nodex.Text, 9))
           nodex.Checked = True
        End If

        '-- label foreground color by status
        If LCase$(nodex.Text) Like "*partial*working*" Then
           nodex.ForeColor = &H80FF&
        ElseIf LCase$(nodex.Text) Like "*not working*" Then
           nodex.ForeColor = vbRed
        ElseIf LCase$(nodex.Text) Like "*working*" Then
           nodex.ForeColor = vbBlue
        End If
      ElseIf line Like "[A-Za-z]*" Then
        If line Like "[0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f] [0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f]" Then
          'do not add game genie codes starting with letters
        ElseIf LCase$(line) Like "carry[ (]*" Then
          'do not add set commands as cheat names
        ElseIf LCase$(line) Like "set *:*" Then
          'do not add set commands as cheat names
        ElseIf LCase$(line) Like "write *:*" Then
          'do not add set commands as cheat names
        ElseIf LCase$(line) Like "insert *:*" Then
          'do not add set commands as cheat names
        ElseIf LCase$(line) Like "delete *:*" Then
          'do not add set commands as cheat names
        ElseIf LCase$(line) Like "search *" Then
          'do not add set commands as cheat names
        ElseIf LCase$(line) Like "copy *" Then
          'do not add set commands as cheat names
        ElseIf LCase$(line) Like "compress *" Then
          'do not add set commands as cheat names
        ElseIf LCase$(line) Like "decompress *" Then
          'do not add set commands as cheat names
        ElseIf LCase$(line) Like "msgbox *" Then
          'do not add set commands as cheat names
        Else
          '-- group label
          If LCase$(line) Like "group:*" Then
            line = Trim$(Mid$(line, 7))
            Group = line
            
            If Group = "\" Or Group = "" Then
              Group = ""
              Set nodex = Nothing
            ElseIf Group Like "*\*" Then
              Data = modVB6.Split(Group, "\")
              line = Data(UBound(Data))
              Set nodex = tvCheats.Nodes.Add(tvCheats.Nodes(Left$(Group, Len(Group) - Len(line) - 1)), tvwChild, Group, line)
              With tvCheats.Nodes(Left$(Group, Len(Group) - Len(line) - 1))
                .Bold = True
                .Expanded = True
              End With
            Else
              Set nodex = tvCheats.Nodes.Add(, , Group, Group)
            End If
            
            nodex.Bold = True
            nodex.Expanded = True
          ElseIf Len(Group) Then
            Set nodex = tvCheats.Nodes.Add(Group, tvwChild, , line)
            nodex.Expanded = True
          Else
            Set nodex = tvCheats.Nodes.Add(, , , line)
            nodex.Expanded = True
          End If
        End If
        
        '-- checkmark cheat on load
        If LCase$(nodex.Text) Like "default:*" Then
          nodex.Text = Trim$(Mid$(nodex.Text, 9))
          nodex.Checked = True
        ElseIf LCase$(nodex.Text) Like "*(required)" Then
          nodex.Checked = True
        End If

        '-- label background color
        If LCase$(nodex.Text) Like "yellow:*" Then
          nodex.Text = Trim$(Mid$(nodex.Text, 8))
          nodex.BackColor = vbYellow
        ElseIf LCase$(nodex.Text) Like "green:*" Then
          nodex.Text = Trim$(Mid$(nodex.Text, 7))
          nodex.BackColor = vbGreen
        ElseIf LCase$(nodex.Text) Like "blue:*" Then
          nodex.Text = Trim$(Mid$(nodex.Text, 6))
          nodex.BackColor = vbBlue
          nodex.ForeColor = vbWhite
        ElseIf LCase$(nodex.Text) Like "red:*" Then
          nodex.Text = Trim$(Mid$(nodex.Text, 5))
          nodex.BackColor = vbRed
          nodex.ForeColor = vbWhite
        ElseIf LCase$(nodex.Text) Like "orange:*" Then
          nodex.Text = Trim$(Mid$(nodex.Text, 8))
          nodex.BackColor = &H80FF&
        ElseIf LCase$(nodex.Text) Like "cyan:*" Then
          nodex.Text = Trim$(Mid$(nodex.Text, 6))
          nodex.BackColor = vbCyan
        ElseIf LCase$(nodex.Text) Like "magenta:*" Then
          nodex.Text = Trim$(Mid$(nodex.Text, 9))
          nodex.BackColor = vbMagenta
        ElseIf LCase$(nodex.Text) Like "gray:*" Then
          nodex.Text = Trim$(Mid$(nodex.Text, 6))
          nodex.BackColor = &HC0C0C0
        End If
        
        '-- label info / checkmarked
        If LCase$(nodex.Text) Like "info:*" Then
           nodex.Bold = True
        ElseIf LCase$(nodex.Text) Like "default:*" Then
           nodex.Text = Trim$(Mid$(nodex.Text, 9))
           nodex.Checked = True
        End If

        '-- label foreground color by status
        If LCase$(nodex.Text) Like "*partial working*" Then
           nodex.ForeColor = &H80FF&
        ElseIf LCase$(nodex.Text) Like "*not working*" Then
           nodex.ForeColor = vbRed
        ElseIf LCase$(nodex.Text) Like "*working*" Then
           nodex.ForeColor = vbBlue
        End If
      End If
    Loop
    Close #fh

    LockWindowUpdate 0&
  End If

  If tvCheats.Nodes.Count = 0 Then
    btnApply.Enabled = False
    tvCheats.Nodes.Add , , , "(There are not cheats not available)"
    tvCheats.Enabled = False
  ElseIf tvCheats.Nodes.Count = 1 Then
    tvCheats.Enabled = True
    tvCheats.Nodes(1).Checked = True
    btnApply.Enabled = True
    btnApply.default = True
  Else
    tvCheats.Enabled = True
    btnApply.Enabled = True
    btnApply.default = True
  End If

  Set tvCheats.SelectedItem = tvCheats.Nodes(1)
  tvCheats.SelectedItem.EnsureVisible
End Sub

Private Sub ApplyPatch(ByVal PatchFile As String, ByVal Path As String, Optional FromLine As Long = 0)
  Dim isactive As Boolean, n As Long, Pointer As Long, hexaddress As String, NotFound As Boolean
  Dim line As String, b As Byte, d As Byte, i As Long, f As Long, c As Long, lline As String
  Dim Address As Long, hexdata As String, fh As Long, operator As Integer, pvalue As Long, IsFromPointer As Boolean
  Dim Filename As String, Files As Collection, fo As Long, pos As Long, ppointer As Long
  Dim StartRange As Long, endrange As Long, Group As String, ReplaceWith As String
  
  Dim Address64 As Currency, c64 As Currency
  
  Dim ntimes As Long, incr_address_by As Long, incr_value_by As Long, r As Long
  Dim datasize As Long, databin As String, searchbin As String, FileSize As Currency
  Dim fromaddress As String, toaddress As String, ltoaddress As Long, Text As String
  Dim varname As String, vars As New Collection, parameter As String
  Dim CarryBytes As Byte, woperator As Integer
  Dim CurrentPatchName As String, Key As String

  Dim rr As Long, parms As String, maddress As Long

  Dim bMsg As Boolean, NumSel As Long
  Dim sum As Currency, bytes() As Byte
  Dim prevline As String, lc As Long

  Dim IsFuse As Boolean, IsTLOU As Boolean

  Const colTitleID = 2
  Const colPARMS As Integer = 8
  On Local Error Resume Next

  bMsg = True

  IsFuse = IsIn(TitleID, "BLUS31040", "BLES01724", "BLES01724", "NPUB30874", "NPUB90815", "NPEB01112", "NPEB90442")
  IsTLOU = IsNaughtyDog(TitleID)

  NumSel = TreeSelCount()

  If tvCheats.Nodes.Count <= 0 Or NumSel <= 0 Then
    Exit Sub
  ElseIf FileExists(PatchFile) Then
    Screen.MousePointer = vbHourglass

    Set Files = New Collection
  
    fo = 0: lc = 0
    isactive = False
    CurrentPatchName = ""
    CarryBytes = 0

    If IsFolder(Path) Then
      Path = NormalizePath(Path)
      
      '-- load all data files to files collection
      Filename = Dir$(Path & "*.*", vbNormal + vbReadOnly + vbHidden + vbSystem + vbArchive)
      Do Until Filename = ""
        Select Case LCase$(Filename)
         Case "param.sfo", "param.pfd", "icon0.png", "pic1.png", "eboot.bin", "eboot.elf", "~files_decrypted_by_pfdtool.txt"
         Case Else
          If LCase$(Filename) Like "*.bak" Then
            'ignore backups
          Else
            Files.Add Path & Filename
          End If
        End Select
        Filename = Dir$()
      Loop
    Else
      Files.Add Path
      Path = ExtractPath(Path)
    End If

    If Path Like "*\" & OutDir & "\" Then
      Path = Left$(Path, Len(Path) - Len(OutDir) - 1)
      ChDrive Path
      ChDir Path
    End If

    fh = FreeFile
    Open PatchFile For Input Shared As #fh
    Do Until EOF(fh)
      prevline = lline
      Line Input #fh, line
      lc = lc + 1

      '-- remove extra spaces
      line = Trim$(line)
      lline = LCase$(line)
      
      hexaddress = ""
      
      '-- remove remarks
      pos = modVB6.InStrRev(line, ";")
      If pos Then
        If line Like "; --- * ---" Then
          Group = Mid$(line, 7, Len(line) - 10)
        ElseIf InStr(line, quote) Then
          line = Trim$(Left$(line, modVB6.InStrRev(line, quote)))
        Else
          line = Trim$(Left$(line, pos - 1))
        End If
      End If

      If lc < FromLine Then
        'ignore
      ElseIf isactive Then
        '*** BSD commands: msgbox / write at / write next / copy / search

        If InStr(lline, "?") Then
          If lline Like "[0-9a-f?][0-9a-f?][0-9a-f?][0-9a-f?][0-9a-f?][0-9a-f?][0-9a-f?][0-9a-f?] [0-9a-f?][0-9a-f?][0-9a-f?][0-9a-f?][0-9a-f?][0-9a-f?][0-9a-f?][0-9a-f?]" Then
            databin = Replace(lline, "?")
            databin = String$(Len(lline) - Len(databin), "?")
            
            Screen.MousePointer = vbDefault

            formWildCard.Filter = databin & "=*"
            formWildCard.Filename = PatchFile
            formWildCard.lblGameName.Caption = formMain.titleids(TitleID)
            formWildCard.lblTitle.Caption = CurrentPatchName
            formWildCard.Show vbModal, Me
            Unload formWildCard
            
            Screen.MousePointer = vbHourglass

            If Len(formMain.wildcard) = 0 Then
              Exit Do
            Else
              line = Replace(line, databin, Left$(formMain.wildcard, Len(databin)))
              lline = Replace(lline, databin, Left$(formMain.wildcard, Len(databin)))
            End If
          End If
        End If
        
        '-- blank line or remarks
        If Len(line) = 0 Or line Like ";*" Then
          GoTo nextcommand
          
        '-- carry(n)
        ElseIf lline Like "carry(*)" Then
          databin = Mid$(line, 7, Len(line) - 7)
          
          CarryBytes = 0
          If databin Like "0x*" Then
            CarryBytes = Abs(ValU(Ox & Mid$(databin, 3)))
          Else
            CarryBytes = Abs(ValU(databin))
          End If
          
          If CarryBytes > 4 Then CarryBytes = 4
          
        ElseIf lline Like "carry *" Then
          databin = Trim$(Mid$(line, 7))

          CarryBytes = 0
          If databin Like "0x*" Then
            CarryBytes = Abs(ValU(Ox & Mid$(databin, 3)))
          Else
            CarryBytes = Abs(ValU(databin))
          End If

          If CarryBytes > 4 Then CarryBytes = 4

        ElseIf lline Like "decompress *" Then
          Filename = Mid$(line, 12)
          If Filename Like """*""" Then Filename = RemoveBrackets(Filename)

          DeleteFile Path & OutDir

          ChDrive Path
          ChDir Path
          
          MkDir Path & OutDir

          If FileExists(Filename) = False Then
            'ignore decompress command
          ElseIf FileExists(AppPath & "offzip_params.ini") Then
            If fo Then Close #fo
            
            fo = FreeFile
            Open AppPath & "offzip_params.ini" For Input Shared As #fo
            Do Until EOF(fo) Or Dir$(Path & OutDir & "\*.*") > ""
              Line Input #fo, parms
              dos.ExecuteCommand quote & AppPath & "offzip.exe" & quote & " " & parms & " " & Filename & " " & OutDir & " 0", ReadDisplay:=False
            Loop
            Close #fo
          Else
            dos.ExecuteCommand quote & AppPath & "offzip.exe" & quote & " -a " & Filename & " " & OutDir & " 0", ReadDisplay:=False
            If Dir$(Path & OutDir & "\*.*") = "" Then
              dos.ExecuteCommand quote & AppPath & "offzip.exe" & quote & " -a -z -15 " & Filename & " " & OutDir & " 0", ReadDisplay:=False
            End If
          End If
          GoTo nextcommand
        ElseIf lline Like "compress *" Then
          If lline Like "*,*" Then
            pos = 1
            Filename = GetParameter(line, pos, "compress ", ",")
            parms = GetParameter(line, pos, ",", "")
          ElseIf FileExists(Path & Mid$(line, 10)) Then
            Filename = Mid$(line, 10)
            parms = "-w 15"
          Else
            Filename = ExtractFile(Files(1))
            parms = "-w 15"
          End If
          
          If InStr(parms, "-") = 0 Then parms = "-w 15"
          
          ChDrive Path
          ChDir Path & OutDir

          databin = Dir$(Path & OutDir & "\*.*")
          Do Until databin = ""
            databin = Left$(databin, InStr(databin, ".") - 1)

            dos.ExecuteCommand quote & AppPath & "packzip.exe" & quote & " " & parms & " -o 0x" & databin & " " & databin & ".dat " & Path & Filename
            databin = Dir$()
          Loop

          ChDir Path

          DeleteFile Path & OutDir
          GoTo nextcommand

        '-- msgbox
        ElseIf lline Like "msgbox *" Then
          hexdata = Trim$(Mid$(line, 8))
          databin = ""

          If LCase$(hexdata) = "filename" Then
            MsgBox Files(1), vbInformation, "Total: " & Files.Count
            GoTo nextcommand
          ElseIf LCase$(hexdata) = "pointer" Then
            MsgBox "0x" & Right$("0000000" & Hex$(Pointer), 8) & " -> decimal (" & Pointer & ")", vbInformation, "Pointer"
            bMsg = False
            GoTo nextcommand
          ElseIf hexdata Like """*""*" Then
            pos = InStr(2, hexdata, quote)
            databin = Mid$(hexdata, 2, pos - 2)
            hexdata = Trim$(Mid$(hexdata, pos + 1))
          End If
          
          hexdata = ParseExpression(vars, hexdata)

          d = 0
          Text = ""
          For i = 1 To Len(hexdata) Step 2
            b = Ox & Mid$(hexdata, i, 2)
            If b > 0 Then
              Text = Text & Chr$(b)
              If b = 10 Or b = 13 Or b = 9 Then
                'still text mode
              ElseIf b < 32 Or b > 127 Then
                'exit from text mode
                d = 1: Exit For
              End If
            End If
          Next
          
          If d Then
            databin = databin & " 0x" & hexdata
          Else
            databin = databin & Text & IIf(Len(Text) > 4 Or Len(hexdata) = 0, "", vbCrLf & "0x" & hexdata)
          End If
          
          If GetAsyncKeyState(vbKeyControl) < 0 Then databin = databin & vbCrLf & "inverted: 0x" & Hex$(Not CLng(Ox & hexdata)) & vbCrLf

          MsgBox modVB6.Replace(databin, "\n", vbCrLf), vbInformation
          bMsg = False
          GoTo nextcommand

        ElseIf lline Like "write * times *:*" Then
          pos = 1
          ntimes = ParseHexNumber(vars, GetParameter(line, pos, "write ", " times ", default:=1))
          maddress = CLng(Ox & ParseAddress(vars, GetParameter(line, pos, "starting at ", " advancing ", " incrementing ", default:=Hex$(Pointer)), Files, Pointer))
          incr_address_by = ParseHexNumber(vars, GetParameter(line, pos, "advancing ", " incrementing ", ":", default:=0))
          incr_value_by = ParseHexNumber(vars, GetParameter(line, pos, "incrementing ", ":", default:=0))

          'write 10 times starting at 0x50 advancing 10 incrementing 2:FFBB
          parms = Mid$(line, pos + 1)

          If parms Like "0x*" Then
            datasize = Len(Mid$(parms, 3))
          ElseIf parms Like "[0-9A-Fa-f]*" Then
            datasize = Len(parms) * IIf(parms Like "*[0-9A-Fa-f]", 1, 0)
          Else
            datasize = Len(ParseExpression(vars, parms))
          End If
          
          If incr_address_by = 0 Then incr_address_by = datasize \ 2

          For rr = 1 To ntimes
            line = "write at 0x" & Hex$(maddress) & ":" & parms
            lline = LCase$(line)
            maddress = maddress + incr_address_by
            If incr_value_by Then
              Select Case datasize
               Case 1 To 2: parms = Right$("0" & Hex$(ParseHexNumber(vars, parms) + incr_value_by), 2)
               Case 3 To 4: parms = Right$("000" & Hex$(ParseHexNumber(vars, parms) + incr_value_by), 4)
               Case 5 To 8: parms = Right$("0000000" & Hex$(ParseHexNumber(vars, parms) + incr_value_by), 8)
               Case Else:   parms = Hex$(ParseHexNumber(vars, parms) + incr_value_by)
                            If Len(parms) Mod 2 Then parms = "0" & parms
              End Select
            End If
            
            GoSub WriteAt
          Next
        '-- write at <address>:value
        ElseIf lline Like "write at *:*" Or lline Like "insert at *:*" Or lline Like "delete at *:*" Then
WriteAt:
          pos = InStr(line, ":")
          Mid$(line, pos, 1) = " "
          
          '-- <address>: [0x]zzzz|(decimal)|[variable]
          If lline Like "insert at *" Then
            operator = 8
            hexaddress = Mid$(line, 11, pos - 11)
          ElseIf lline Like "delete at *" Then
            operator = 9
            hexaddress = Mid$(line, 11, pos - 11)
          Else
            operator = 0
            hexaddress = Mid$(line, 10, pos - 10)
          End If
          
          hexaddress = ParseAddress(vars, hexaddress, Files, Pointer)

          '-- value: [variable]|"text"|hex bytes
          hexdata = Trim$(Mid$(line, pos + 1))
          parameter = LCase$(hexdata)
          
          If parameter Like "or:*" Then
            operator = 1
            hexdata = Trim$(Mid$(hexdata, 4))
            hexdata = ParseExpression(vars, hexdata)
          ElseIf parameter Like "and:*" Then
            operator = 2
            hexdata = Trim$(Mid$(hexdata, 5))
            hexdata = ParseExpression(vars, hexdata)
          ElseIf parameter Like "xor:*" Then
            operator = 3
            hexdata = Trim$(Mid$(hexdata, 5))
            hexdata = ParseExpression(vars, hexdata)
          ElseIf parameter Like "eqv:*" Then
            operator = 4
            hexdata = Trim$(Mid$(hexdata, 5))
            hexdata = ParseExpression(vars, hexdata)
          ElseIf parameter Like "imp:*" Then
            operator = 5
            hexdata = Trim$(Mid$(hexdata, 5))
            hexdata = ParseExpression(vars, hexdata)
          ElseIf parameter Like "revimp:*" Then
            operator = 6
            hexdata = Trim$(Mid$(hexdata, 8))
            hexdata = ParseExpression(vars, hexdata)
          ElseIf parameter Like "add:*" Then
            operator = 7
            hexdata = Trim$(Mid$(hexdata, 5))
            hexdata = ParseExpression(vars, hexdata)

            If Mid$(hexdata, 2) Like "(*)" Then
              hexdata = Hex$(-CLng(hexdata))
            ElseIf hexdata Like "-*" Then
              If Mid$(hexdata, 2) Like "(*)" Then
                hexdata = Hex$(CLng(Mid$(hexdata, 2)))
              Else
                hexdata = Hex$(-CLng(Ox & Mid$(hexdata, 2)))
              End If
            End If
          ElseIf parameter Like "until *" Then
            ntimes = 1
            If hexdata Like "*:*" Then
              For i = Len(hexdata) To 1 Step -1
                If Mid$(hexdata, i, 1) = ":" Then
                  ntimes = ValU(Mid$(hexdata, i + 1))
                  hexdata = Left$(hexdata, i - 1)
                  Exit For
                End If
              Next
            End If
          
            hexdata = Trim$(Mid$(hexdata, 7))
            If hexdata Like """*""" Then
              searchbin = RemoveBrackets(hexdata)
            ElseIf LCase$(hexdata) Like "unicode(""*"")" Then
              searchbin = StrConv(Mid$(hexdata, 10, Len(hexdata) - 11), vbUnicode)
            Else
              hexdata = ParseHexNumber(vars, hexdata)
              If hexdata Like "0x*" Then
                hexdata = Mid$(hexdata, 3)
              End If
              
              searchbin = ""
              For i = 1 To Len(hexdata) Step 2
                searchbin = searchbin & Chr$(Ox & Mid$(hexdata, i, 2))
              Next
            End If
  
            If Len(searchbin) Then
              For f = 1 To Files.Count
                If FileExists(Files(f)) Then
                  If fo Then Close #fo
                  fo = FreeFile
                  Open Files(f) For Binary Shared As #fo
                  databin = AllocString(LOF(fo))
                  Get #fo, , databin
                  Close #fo
                  fo = 0
                  
                  c = InStr(ppointer + 1, databin, searchbin)

                  If c Then
                    For i = 1 To ntimes - 1
                      c = InStr(c + Len(searchbin), databin, searchbin)
                    Next
                      
                    If c > ppointer Then
                      hexdata = Hex$(c - ppointer - 1)
                      If c - ppointer - 1 <= 0 Then GoTo nextcommand
                    Else
                      GoTo nextcommand
                    End If
                  Else
                    GoTo nextcommand
                  End If
                End If
              Next
            Else
              NotFound = False
            End If
          ElseIf hexdata Like "repeat(*" Then
            line = "set [" & operator & "x" & hexaddress & "]:" & hexdata
            lline = LCase$(line)
            GoTo parseline
          Else
            hexdata = ParseExpression(vars, hexdata)
          End If

          '-- BSD hex format: 0x<address> <hex bytes>
          line = operator & "x" & hexaddress & " " & hexdata
          lline = LCase$(line)
          NotFound = False
          
        '-- write next <address>:value
        ElseIf lline Like "write next *:*" Or lline Like "insert next *:*" Or lline Like "delete next *:*" Then
          If NotFound Then
            'ignore write
          Else
            pos = InStr(line, ":")
            Mid$(line, pos, 1) = " "

            '-- <address>: [0x]zzzz|(decimal)|[variable]
            If lline Like "insert next *" Then
              operator = 8
              hexaddress = Mid$(line, 13, pos - 13)
            ElseIf lline Like "delete next *" Then
              operator = 9
              hexaddress = Mid$(line, 13, pos - 13)
            Else
              operator = 0
              hexaddress = Mid$(line, 12, pos - 12)
            End If

            hexaddress = Hex$(Pointer + CLng(Ox & ParseAddress(vars, hexaddress, Files, Pointer, ppointer)))

            '-- value: [variable]|"text"|hex bytes
            hexdata = Trim$(Mid$(line, pos + 1))
            parameter = LCase$(hexdata)

            If parameter Like "or:*" Then
              operator = 1
              hexdata = Trim$(Mid$(hexdata, 4))
              hexdata = ParseExpression(vars, hexdata)
            ElseIf parameter Like "and:*" Then
              operator = 2
              hexdata = Trim$(Mid$(hexdata, 5))
              hexdata = ParseExpression(vars, hexdata)
            ElseIf parameter Like "xor:*" Then
              operator = 3
              hexdata = Trim$(Mid$(hexdata, 5))
              hexdata = ParseExpression(vars, hexdata)
            ElseIf parameter Like "eqv:*" Then
              operator = 4
              hexdata = Trim$(Mid$(hexdata, 5))
              hexdata = ParseExpression(vars, hexdata)
            ElseIf parameter Like "imp:*" Then
              operator = 5
              hexdata = Trim$(Mid$(hexdata, 5))
              hexdata = ParseExpression(vars, hexdata)
            ElseIf parameter Like "revimp:*" Then
              operator = 6
              hexdata = Trim$(Mid$(hexdata, 8))
              hexdata = ParseExpression(vars, hexdata)
            ElseIf parameter Like "add:*" Then
              operator = 7
              hexdata = Trim$(Mid$(hexdata, 5))
              hexdata = ParseExpression(vars, hexdata)

              If Mid$(hexdata, 2) Like "(*)" Then
                hexdata = Hex$(-CLng(hexdata))
              ElseIf parameter Like "-*" Then
                If Mid$(hexdata, 2) Like "(*)" Then
                  hexdata = Hex$(CLng(Mid$(hexdata, 2)))
                Else
                  hexdata = Hex$(-CLng(Ox & Mid$(hexdata, 2)))
                End If
              End If
            ElseIf parameter Like "until *" Then
              ntimes = 1
              If hexdata Like "*:*" Then
                For i = Len(hexdata) To 1 Step -1
                  If Mid$(hexdata, i, 1) = ":" Then
                    ntimes = ValU(Mid$(hexdata, i + 1))
                    hexdata = Left$(hexdata, i - 1)
                    Exit For
                  End If
                Next
              End If
            
              hexdata = Trim$(Mid$(hexdata, 7))
              If hexdata Like """*""" Then
                searchbin = RemoveBrackets(hexdata)
              ElseIf LCase$(hexdata) Like "unicode(""*"")" Then
                searchbin = StrConv(Mid$(hexdata, 10, Len(hexdata) - 11), vbUnicode)
              Else
                hexdata = ParseHexNumber(vars, hexdata)
                If hexdata Like "0x*" Then
                  hexdata = Mid$(hexdata, 3)
                End If
                
                searchbin = ""
                For i = 1 To Len(hexdata) Step 2
                  searchbin = searchbin & Chr$(Ox & Mid$(hexdata, i, 2))
                Next
              End If
    
              If Len(searchbin) Then
                For f = 1 To Files.Count
                  If FileExists(Files(f)) Then
                    If fo Then Close #fo
                    fo = FreeFile
                    Open Files(f) For Binary Shared As #fo
                    databin = AllocString(LOF(fo))
                    Get #fo, , databin
                    Close #fo
                    fo = 0

                    c = InStr(ppointer + 1, databin, searchbin)

                    If c Then
                      For i = 1 To ntimes - 1
                        c = InStr(c + Len(searchbin), databin, searchbin)
                      Next
                        
                      If c > ppointer Then
                        hexdata = Hex$(c - ppointer - 1)
                        If c - ppointer - 1 <= 0 Then GoTo nextcommand
                      Else
                        GoTo nextcommand
                      End If
                    Else
                      GoTo nextcommand
                    End If
                  End If
                Next
              Else
                NotFound = False
              End If

            ElseIf hexdata Like "repeat(*" Then
              line = "set [" & operator & "x" & hexaddress & "]:" & hexdata
              lline = LCase$(line)
              GoTo parseline
            Else
              hexdata = ParseExpression(vars, hexdata)
            End If
            
            '-- BSD hex format: 0x<address> <hex bytes>
            line = operator & "x" & Hex$(ppointer) & " " & hexdata
            NotFound = False
          End If
        
        '-- copy <from address>:<to address>:<size>
        ElseIf lline Like "copy *:*" Then
          pos = InStr(line, ":")
          If pos = 0 Then pos = Len(line) + 1
          
          fromaddress = Trim$(Mid$(line, 6, pos - 6))
          
          '-- from address: [0x]zzzz|(decimal)
          databin = fromaddress
          databin = ParseAddress(vars, databin, Files, Pointer)
          Address = CLng(Ox & databin)

          '-- to address[:size]
          toaddress = Trim$(Mid$(line, pos + 1))

          '-- data size
          datasize = 1
          pos = InStr(toaddress, ":")
          If pos Then
            databin = Mid$(toaddress, pos + 1)
            If Left$(databin, 2) = "0x" Then
              Mid$(databin, 1, 2) = Ox
            ElseIf databin Like "*[A-Fa-f]*" Then
              databin = Ox & databin
            ElseIf databin Like "(*)" Then
              databin = RemoveBrackets(databin)
            End If
            datasize = CLng(databin)
            toaddress = Trim$(Left$(toaddress, pos - 1))
          End If

          '-- to address: [0x]zzzz|(decimal)
          databin = toaddress
          databin = ParseAddress(vars, databin, Files, Pointer)
          ltoaddress = CLng(Ox & databin)

          '-- copy data from first file
          If Files.Count > 0 Then
            If fo Then Close #fo
            fo = FreeFile
            Open Files(1) For Binary Shared As #fo
            If Address >= 0 And Address + databin <= LOF(fo) Then
              databin = AllocString(datasize)
              Get #fo, Address + 1, databin
              hexdata = BinToHex(databin)
    
              '-- BSD hex format: 0x<address> <hex bytes>
              line = "0x" & Hex$(ltoaddress) & " " & hexdata
              lline = LCase$(line)
            Else
              line = ""
            End If
            Close #fo
            fo = 0
          Else
            line = ""
          End If

        '-- search "text"|<hex bytes>:<times>
        '-- search next (<pointer>) "text"|<hex bytes>:<times>
        '-- search "text"|<hex bytes> replace with "text"|<hex bytes>:<times>
        '-- search next (<pointer>) "text"|<hex bytes> replace with "text"|<hex bytes>:<times>
        ElseIf lline Like "search *" Or lline Like "search next *" Then
          If lline Like "search next *" Then
            If NotFound Then
              GoTo nextcommand
            ElseIf lline Like "search next (*) *" Then
              pos = InStr(line, ")")
              hexdata = Mid$(line, 14, pos - 14)
              If hexdata Like "0x*" Then
                r = Pointer + CLng(Ox & Trim$(Mid$(hexdata, 3)))
              ElseIf hexdata Like "*[A-Fa-f]*" Then
                r = Pointer + CLng(Ox & Trim$(hexdata))
              ElseIf hexdata Like VariableMask Then
                r = Pointer + CLng(Ox & GetVariable(vars, hexdata))
              Else
                r = Pointer + CLng(hexdata)
              End If
              line = "search " & Trim$(Mid$(line, pos + 1))
              lline = LCase$(line)
            Else
              r = Pointer + Len(searchbin)
              line = "search " & Trim$(Mid$(line, 13))
              lline = LCase$(line)
            End If
          Else
            r = 0
          End If

          '----- replace with -----
          ReplaceWith = ""
          pos = InStr(1, line, " replace with ", vbTextCompare)

          If pos Then
            hexdata = Mid$(line, pos + 14)
            i = modVB6.InStrRev(hexdata, ":")
            If i = 0 Then i = Len(hexdata) + 1
            hexdata = Trim$(Left$(hexdata, i - 1))
          
            If hexdata Like """*""" Then
              ReplaceWith = RemoveBrackets(hexdata)
            ElseIf hexdata Like VariableMask Then
              ReplaceWith = HexToBin(GetVariable(vars, hexdata))
            ElseIf LCase$(hexdata) Like "unicode(""*"")" Then
              ReplaceWith = StrConv(Mid$(hexdata, 10, Len(hexdata) - 11), vbUnicode)
            Else
              If hexdata Like "0x*" Then hexdata = Mid$(hexdata, 3)
              For i = 1 To Len(hexdata) Step 2
                ReplaceWith = ReplaceWith & Chr$(Ox & Mid$(hexdata, i, 2))
              Next
            End If
          Else
            pos = modVB6.InStrRev(line, ":")
          End If
          '------------------
          
          NotFound = True
          
          If pos = 0 Then pos = Len(line) + 1
          
          hexdata = Trim$(Mid$(line, 8, pos - 8))
          If hexdata Like """*""" Then
            searchbin = RemoveBrackets(hexdata)
          ElseIf hexdata Like VariableMask Then
            searchbin = HexToBin(GetVariable(vars, hexdata))
          ElseIf LCase$(hexdata) Like "unicode(""*"")" Then
            searchbin = StrConv(Mid$(hexdata, 10, Len(hexdata) - 11), vbUnicode)
          Else
            searchbin = ""
            If hexdata Like "0x*" Then hexdata = Mid$(hexdata, 3)
            For i = 1 To Len(hexdata) Step 2
              searchbin = searchbin & Chr$(Ox & Mid$(hexdata, i, 2))
            Next
          End If

          If Len(searchbin) Then
            pos = modVB6.InStrRev(line, ":")
          
            For f = 1 To Files.Count
              If FileExists(Files(f)) Then
                If fo Then Close #fo
                fo = FreeFile
                Open Files(f) For Binary Shared As #fo
                databin = AllocString(LOF(fo))
                Get #fo, , databin
                Close #fo
                fo = 0
                
                If Len(ReplaceWith) Then
                  c = 0
                  Pointer = InStr(r + 1, databin, searchbin)
                  ntimes = ValU(Mid$(line, pos + 1))
                  Do While Pointer > 0
                    If Len(searchbin) = Len(ReplaceWith) Then
                      Mid$(databin, Pointer, Len(ReplaceWith)) = ReplaceWith
                    Else
                      databin = Left$(databin, Pointer - 1) & ReplaceWith & Mid$(databin, Pointer + Len(searchbin))
                    End If

                    c = c + 1
                    If ntimes > 0 And c >= ntimes Then Exit Do

                    Pointer = InStr(Pointer + Len(ReplaceWith), databin, searchbin)
                  Loop
                  
                  If (c > 0) And (Len(databin) > 0) Then
                    fo = FreeFile
                    Open Files(f) For Binary Shared As #fo
                    Put #fo, 1, databin
                    Close #fo
                    fo = 0
                  End If
                Else
                  Pointer = InStr(r + 1, databin, searchbin)
  
                  For i = 1 To ValU(Mid$(line, pos + 1)) - 1
                    c = InStr(Pointer + 1, databin, searchbin)
                    If c Then
                      Pointer = c
                    Else
                      Pointer = 0
                      Exit For
                    End If
                  Next
                End If

                If Pointer Then Pointer = Pointer - 1: NotFound = False: GoTo nextcommand
              End If
            Next
          Else
            NotFound = False
          End If
          GoTo nextcommand
        End If
      End If

      '---------------------------------
      'GameGenie Code Types: (T=0 absolute  T=8 relative to pointer)
      '0TXXXXXX 000000YY = 8Bit Write
      '1TXXXXXX 0000YYYY = 16Bit Write
      '2TXXXXXX YYYYYYYY = 32Bit Write
      '
      'Example:
      '12345678901234567
      '28BF4FD5 646951FB
      If isactive = False Then
        'skip GameGenie Code
      ElseIf isactive And line Like "0[08][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f] [0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f]" Then
        '0TXXXXXX 000000YY = 8Bit Write  (T=0 absolute  T=8 relative to pointer)
        operator = CInt(Ox & Mid$(line, 2, 1))
        IsFromPointer = (operator = 8)
        If IsFromPointer Then
          ppointer = Pointer + CLng(Ox & Mid$(line, 3, 6))
          hexaddress = Right$(String$(7, "0") & Hex$(ppointer), 8)
          'pointer = pointer + 1
        Else
          hexaddress = Mid$(line, 3, 6)
          'pointer = Ox & hexaddress
        End If
        
        hexdata = Mid$(line, 16, 2)
        line = "0x" & hexaddress & " " & hexdata
        lline = LCase$(line)
      ElseIf isactive And line Like "1[08][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f] [0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f]" Then
        '1TXXXXXX 0000YYYY = 16Bit Write  (T=0 absolute  T=8 relative to pointer)
        operator = CInt(Ox & Mid$(line, 2, 1))
        IsFromPointer = (operator = 8)
        If IsFromPointer Then
          ppointer = Pointer + CLng(Ox & Mid$(line, 3, 6))
          hexaddress = Right$(String$(7, "0") & Hex$(ppointer), 8)
          'pointer = pointer + 2
        Else
          hexaddress = Mid$(line, 3, 6)
          'pointer = Ox & hexaddress
        End If
        
        hexdata = Mid$(line, 14, 4)
        line = "0x" & hexaddress & " " & hexdata
        lline = LCase$(line)
      ElseIf isactive And line Like "2[08][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f] [0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f]" Then
        '2TXXXXXX YYYYYYYY = 32Bit Write (T=0 absolute  T=8 relative to pointer)
        operator = CInt(Ox & Mid$(line, 2, 1))
        IsFromPointer = (operator = 8)
        If IsFromPointer Then
          ppointer = Pointer + CLng(Ox & Mid$(line, 3, 6))
          hexaddress = Right$(String$(7, "0") & Hex$(ppointer), 8)
          'pointer = pointer + 4
        Else
          hexaddress = Mid$(line, 3, 6)
          'pointer = Ox & hexaddress
        End If
        
        hexdata = Mid$(line, 10, 8)
        line = "0x" & hexaddress & " " & hexdata
        lline = LCase$(line)
      ElseIf isactive And line Like "7[012][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f] [0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f]" Then
        '7TXXXXXX YYYYYYYY = Add Write (T: 0=1byte/1=2byte/2=4byte)
        operator = CInt(Ox & Mid$(line, 2, 1))
        hexaddress = Mid$(line, 3, 6)
        Select Case operator
         Case 0: hexdata = Right$(Mid$(line, 10, 8), 2)
         Case 1: hexdata = Right$(Mid$(line, 10, 8), 4)
         Case 2: hexdata = Mid$(line, 10, 8)
        End Select

        line = "7x" & hexaddress & " " & hexdata
        lline = LCase$(line)
      ElseIf isactive And line Like "4[01289A][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f] [0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f]" Then
        'Serial Write
        '4TXXXXXX YYYYYYYY
        '4NNNWWWW VVVVVVVV
      
        '  XXXXXX = Offset to start with
        'YYYYYYYY = 8/16/32-Bit value to start with
        '     NNN = Number of times to repeat
        '    WWWW = Increase address by (in bytes)
        'VVVVVVVV = Increase value by

        'T = Bit Size
        '     0 = 8-Bit from start of the data
        '     1 = 16-Bit from start of the data
        '     2 = 32-Bit from start of the data
        '     8 = 8-Bit from found from a search
        '     9 = 16-Bit from found from a search
        '     A = 32-Bit from found from a search
     
        operator = CInt(Ox & Mid$(line, 2, 1))
        Select Case operator
         Case 0, 1, 2
          hexaddress = Mid$(line, 3, 6)
          'pointer = Ox & hexaddress
         Case Else '"8", "9", "A"
          ppointer = Pointer + CLng(Ox & Mid$(line, 3, 6))
          hexaddress = Right$(String$(7, "0") & Hex$(ppointer), 8)
        End Select

        hexdata = Mid$(line, 10, 8)

        line = ""
        Line Input #fh, line
        line = Trim$(line)
        
        If line Like "4[0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f] [0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f]" Then
          ntimes = CLng(Ox & Mid$(line, 2, 3))
          incr_address_by = CLng(Ox & Mid$(line, 5, 4))
          incr_value_by = CLng(Ox & Mid$(line, 10, 8))
          
          If Left$(hexdata, 2) = "0x" Then
            hexdata = Mid$(hexdata, 3)
          End If
          
          Address = -1 'exception address
          Address = CLng(Ox & hexaddress)

          If Address >= 0 Then
            For f = 1 To Files.Count
              If FileExists(Files(f)) Then
                If fo = 0 Then
                  fo = FreeFile
                  Open Files(f) For Binary Shared As #fo
                End If

                FileSize = LOF(fo)

                For r = 1 To ntimes
                  Select Case operator
                   Case 0, 8
                      c = 0
                      'pointer = Address
                      For i = 7 To 8 Step 2
                        b = Ox & Mid$(hexdata, i, 2)
                        c = c + 1
                        If Address + c <= FileSize Then Put #fo, Address + c, b
                      Next
                      'pointer = pointer + 1
                   Case 1, 9
                      c = 0
                      'pointer = Address
                      For i = 5 To 8 Step 2
                        b = Ox & Mid$(hexdata, i, 2)
                        c = c + 1
                        If Address + c <= FileSize Then Put #fo, Address + c, b
                      Next
                      'pointer = pointer + 2
                   Case 2, &HA
                      c = 0
                      'pointer = Address
                      For i = 1 To 8 Step 2
                        b = Ox & Mid$(hexdata, i, 2)
                        c = c + 1
                        If Address + c <= FileSize Then Put #fo, Address + c, b
                      Next
                      'pointer = pointer + 4
                  End Select

                  Address = Address + incr_address_by

                  If incr_value_by Then
                    hexdata = Right$("0000000" & Hex$(CLng(Ox & hexdata) + incr_value_by), 8)
                  End If
                Next
  
                If Files.Count = 1 Then
                  'keep file open
                Else
                  Close #fo
                  fo = 0
                End If
              End If
            Next
          End If 'address >= 0
        End If '4NNNWWWW VVVVVVVV
      ElseIf isactive And line Like "5[08][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f] [0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f]" Then
        'Copy Code
        '5TXXXXXX ZZZZZZZZ  <- XXXXXX = from address  ZZZZZZZZ = data size
        '5TYYYYYY 00000000  <- YYYYYY = to address
        operator = CInt(Ox & Mid$(line, 2, 1))
        IsFromPointer = (operator = 8)
        If IsFromPointer Then
          ppointer = Pointer + CLng(Ox & Mid$(line, 3, 6))
          hexaddress = Right$(String$(7, "0") & Hex$(ppointer), 8)
        Else
          hexaddress = Mid$(line, 3, 6)
          'pointer = Ox & hexaddress
        End If
        
        fromaddress = hexaddress
        datasize = CLng(Ox & Mid$(line, 10, 8))

        line = ""
        Line Input #fh, line
        line = Trim$(line)
        
        If line Like "5[0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f] [0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f]" Then
          operator = CInt(Ox & Mid$(line, 2, 1))
          IsFromPointer = (operator = 8)
          If IsFromPointer Then
            ppointer = Pointer + CLng(Ox & Mid$(line, 3, 6))
            hexaddress = Right$(String$(7, "0") & Hex$(ppointer), 8)
            'pointer = pointer + 1
          Else
            hexaddress = Mid$(line, 3, 6)
            'pointer = Ox & hexaddress
          End If
          
          toaddress = hexaddress

          If datasize > 0 Then
            For f = 1 To Files.Count
              If FileExists(Files(f)) Then
                If fo Then Close #fo: fo = 0

                If fo = 0 Then
                  Address = -1
                  Address = CLng(Ox & fromaddress)

                  FileSize = FileLen(Files(f))

                  If Address >= 0 And Address <= FileSize - datasize Then
                    ltoaddress = -1
                    ltoaddress = CLng(Ox & toaddress)
                    If ltoaddress >= 0 And ltoaddress <= FileSize - datasize Then
                      fo = FreeFile
                      Open Files(f) For Binary Shared As #fo
                      For c = 1 To datasize
                        Get #fo, Address + c, b
                        Put #fo, ltoaddress + c, b
                      Next
                      'pointer = ltoaddress + datasize
                    End If
                  End If
                End If

                If Files.Count = 1 Then
                  'keep file open
                Else
                  Close #fo
                  fo = 0
                End If
              
                Exit For
              End If
            Next
          End If
        End If
      
      '--------------------------------
      '6TWX0Y0Z VVVVVVVV <- Code Type 6
      '--------------------------------
      '6 = Type 6: Pointer codes
      'T = Data size of VVVVVVVV: 0:8bit, 1:16bit, 2:32bit, search-> 8:8bit, 9:16bit, A:32bit
      'W = operator:
      '      0 = Read "address" from file
      '      1X = Move pointer from obtained address ?? (X = 0:add, 1:substract, 2:multiply)
      '      2X = Move pointer ?? (X = 0:add, 1:substract, 2:multiply)
      '      4X = Write value: X=0 at read address, X=1 at pointer address
      'Y = flag relative to read add (very tricky to understand)
      'Z = flag relative to current pointer (very tricky to understand)
      'v = Data

      '------------------------
      
      '-- 6T0OZZZZ XXXXXXXX  <- Pointer Code: Read offset from address XXXXXXXXX
      ElseIf isactive And line Like "6[0-289Aa]0[0-2]0[0-2]0[0-2] [0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f]" Then
        
        If NotFound And IsIn(Mid$(line, 2, 1), "8", "9", "A", "a") Then GoTo nextcommand

        'T = Bit Size
        Select Case Mid$(line, 2, 1)
         Case "0", "8": datasize = 1 '8 bit
         Case "1", "9": datasize = 2 '16 bit
         Case "2", "A", "a": datasize = 4 '32 bit
        End Select

        incr_address_by = CLng(Ox & Right$(line, 8))

        If IsIn(Mid$(line, 2, 1), "8", "9", "A", "a") Then
          incr_address_by = Pointer + incr_address_by
        End If
        
        operator = Mid$(line, 4, 1) '0=none, 1=add, 2=multiply
        IsFromPointer = Mid$(line, 6, 1) '0=absolute, 1=pointer

        If fo Then Close #fo: fo = 0

        If IsFromPointer Then
          Select Case operator
           Case 0: Pointer = incr_address_by
           Case 1: Pointer = (pvalue + incr_address_by)
           Case 2: Pointer = (pvalue * incr_address_by)
           Case 3: Pointer = (pvalue - incr_address_by)
          End Select
        Else
          Select Case operator
           Case 0: Pointer = incr_address_by
           Case 1: Pointer = Pointer + incr_address_by
           Case 2: Pointer = Pointer * incr_address_by
           Case 3: Pointer = Pointer - incr_address_by
          End Select
        End If

        Select Case Mid$(line, 8, 1)
         Case 0: pvalue = Pointer
         Case 1: pvalue = Pointer + pvalue
        End Select

        fo = FreeFile
        Open Files(1) For Binary Shared As #fo
        databin = AllocString(datasize)
        Get #fo, pvalue + 1, databin
        Close #fo
        fo = 0

        hexdata = BinToHex(databin)
        
        pvalue = CLng(Ox & hexdata)
        GoTo nextcommand
      
      '-- 6T1ZZZZZ XXXXXXXX  <- Pointer Code: Address operations XXXXXXXXX
      ElseIf isactive And line Like "6[0-289Aa]1[0-2]0[0-2]0[0-2] [0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f]" Then
        
        If NotFound And IsIn(Mid$(line, 2, 1), "8", "9", "A", "a") Then GoTo nextcommand
        
        hexdata = Right$(line, 8)

        'T = Bit Size
        Select Case Mid$(line, 2, 1)
         Case "0", "8": datasize = 1 '8 bit
                 hexdata = Right$(hexdata, 2)

         Case "1", "9": datasize = 2 '16 bit
                 hexdata = Right$(hexdata, 4)

         Case "2", "A", "a": datasize = 4 '32 bit
        End Select

        operator = Mid$(line, 4, 1) '0=none, 1=add, 2=multiply, 3=substract?
        IsFromPointer = Mid$(line, 6, 1) '0=absolute, 1=pointer

        incr_address_by = 0
        incr_address_by = CLng(Ox & hexdata)

        If IsFromPointer Then
          Select Case operator
           Case 0
                   pvalue = Pointer + (pvalue + incr_address_by)
           Case 1
                   pvalue = Pointer - (pvalue + incr_address_by)
           Case 2
                   pvalue = Pointer + (pvalue * incr_address_by)
          End Select
        Else
          Select Case operator
           Case 0
                   pvalue = (pvalue + incr_address_by)
           Case 1
                   pvalue = (pvalue - incr_address_by)
           Case 2
                   pvalue = (pvalue * incr_address_by)
          End Select
        End If

        Select Case Mid$(line, 8, 1)
         Case 0
           Pointer = pvalue
         Case 1
           Pointer = Pointer + pvalue
           pvalue = 0
        End Select

        GoTo nextcommand

      '-- 6T1ZZZZZ XXXXXXXX  <- Pointer Code: Set Pointer operation XXXXXXXXX from pointer + ZZZZZ
      ElseIf isactive And line Like "6[0-289Aa]2[0-2]0[0-2]0[0-2] [0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f]" Then
        
        If NotFound And IsIn(Mid$(line, 2, 1), "8", "9", "A", "a") Then GoTo nextcommand
        
        hexdata = Right$(line, 8)

        'T = Bit Size
        Select Case Mid$(line, 2, 1)
         Case "0", "8": datasize = 1 '8 bit
                 hexdata = Right$(hexdata, 2)

         Case "1", "9": datasize = 2 '16 bit
                 hexdata = Right$(hexdata, 4)

         Case "2", "A", "a": datasize = 4 '32 bit
        End Select

        operator = Mid$(line, 4, 1) '0=add, 1=substract, 2=multiply
        IsFromPointer = Mid$(line, 6, 1) '0=absolute, 1=pointer

        incr_address_by = 0
        incr_address_by = CLng(Ox & hexdata)

        If IsFromPointer Then
          Select Case operator
           Case 0
                   pvalue = Pointer + (pvalue + incr_address_by)
           Case 1
                   pvalue = Pointer - (pvalue + incr_address_by)
           Case 2
                   pvalue = Pointer + (pvalue * incr_address_by)
          End Select
        Else
          Select Case operator
           Case 0
                   pvalue = (pvalue + incr_address_by)
           Case 1
                   pvalue = (pvalue - incr_address_by)
           Case 2
                   pvalue = (pvalue * incr_address_by)
          End Select
        End If

        Select Case Mid$(line, 8, 1)
         Case 0
           Pointer = pvalue
         Case 1
           Pointer = Pointer + pvalue
           pvalue = 0
        End Select

        GoTo nextcommand

      '-- 6A4ZZZZZ XXXXXXXX  <- Pointer Code: Set Pointer operation XXXXXXXXX from search pointer + ZZZZZ
      ElseIf isactive And line Like "6[89Aa]4[0-2]0000 [0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f]" Then
        hexdata = Right$(line, 8)

        If NotFound Then GoTo nextcommand

        'T = Bit Size
        Select Case Mid$(line, 2, 1)
         Case "8": datasize = 1 '8 bit
                 hexdata = Right$(hexdata, 2)

         Case "1", "9": datasize = 2 '16 bit
                 hexdata = Right$(hexdata, 4)

         Case "2", "A", "a": datasize = 4 '32 bit
        End Select

        operator = Mid$(line, 4, 1) '0:absolute, 1:add, 2:multiply, 3:substract

        incr_address_by = 0
        incr_address_by = CLng(Ox & hexdata)

        Select Case operator
         Case 0: Pointer = incr_address_by
         Case 1: Pointer = (pvalue + incr_address_by)
         Case 2: Pointer = (pvalue * incr_address_by)
         Case 3: Pointer = (pvalue - incr_address_by)
        End Select
        
        GoTo nextcommand

      '-- 6T4ZZZZZ XXXXXXXX  <- Pointer Code: Write value XXXXXXXXX from pointer + ZZZZZ
      ElseIf isactive And line Like "6[0-2]4[0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f] [0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f]" Then
        
        If NotFound And IsIn(Mid$(line, 2, 1), "8", "9", "A", "a") Then GoTo nextcommand
        
        hexdata = Right$(line, 8)

        'T = Bit Size
        Select Case Mid$(line, 2, 1)
         Case "0", "8": datasize = 1 '8 bit
                 hexdata = Right$(hexdata, 2)

         Case "1", "9": datasize = 2 '16 bit
                 hexdata = Right$(hexdata, 4)

         Case "2", "A", "a": datasize = 4 '32 bit
        End Select

        operator = Mid$(line, 4, 1) '0=from address, 1:from pointer
        IsFromPointer = Mid$(line, 6, 1) '0=absolute, 1=pointer

        incr_address_by = -1

        If IsFromPointer Then
          Select Case operator
           Case 0: incr_address_by = pvalue
           Case 1: incr_address_by = Pointer + pvalue
          End Select
        Else
          Select Case operator
           Case 0: incr_address_by = Pointer
           Case 1: incr_address_by = Pointer + pvalue
          End Select
        End If

        If incr_address_by <= -1 Then GoTo nextcommand

        hexaddress = Hex$(incr_address_by)
        line = "0x" & hexaddress & " " & hexdata
'
'      '-- 6T0OZZZZ XXXXXXXX  <- Pointer Code: Read offset from address XXXXXXXXX
'      ElseIf isactive And line Like "6[0-2]0[0-2]0[0-2]0[0-2] [0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f]" Then
'        'T = Bit Size
'        Select Case Mid$(line, 2, 1)
'         Case 0: datasize = 1 '8 bit
'         Case 1: datasize = 2 '16 bit
'         Case 2: datasize = 4 '32 bit
'        End Select
'
'        operator = Mid$(line, 4, 1) '0=none, 1=add, 2=multiply
'        IsFromPointer = Mid$(line, 6, 1) '0=absolute, 1=pointer
'
'        If fo Then Close #fo: fo = 0
'
'        incr_address_by = CLng(Ox & Right$(line, 8))
'
'        If IsFromPointer Then
'          Select Case operator
'           Case 0: Pointer = incr_address_by
'           Case 1: Pointer = (pvalue + incr_address_by)
'           Case 2: Pointer = (pvalue * incr_address_by)
'           Case 3: Pointer = (pvalue - incr_address_by)
'          End Select
'        Else
'          Select Case operator
'           Case 0: Pointer = incr_address_by
'           Case 1: Pointer = Pointer + incr_address_by
'           Case 2: Pointer = Pointer * incr_address_by
'           Case 3: Pointer = Pointer - incr_address_by
'          End Select
'        End If
'
'        Select Case Mid$(line, 8, 1)
'         Case 0: pvalue = Pointer
'         Case 1: pvalue = Pointer + pvalue
'        End Select
'
'        fo = FreeFile
'        Open Files(1) For Binary Shared As #fo
'        databin = AllocString(datasize)
'        Get #fo, pvalue + 1, databin
'        Close #fo
'        fo = 0
'
'        hexdata = BinToHex(databin)
'
'        pvalue = CLng(Ox & hexdata)
'        GoTo nextcommand
'
'      '-- 6T1ZZZZZ XXXXXXXX  <- Pointer Code: Address operations XXXXXXXXX
'      ElseIf isactive And line Like "6[0-2]1[0-2]0[0-2]0[0-2] [0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f]" Then
'        hexdata = Right$(line, 8)
'
'        'T = Bit Size
'        Select Case Mid$(line, 2, 1)
'         Case 0: datasize = 1 '8 bit
'                 hexdata = Right$(hexdata, 2)
'
'         Case 1: datasize = 2 '16 bit
'                 hexdata = Right$(hexdata, 4)
'
'         Case 2: datasize = 4 '32 bit
'        End Select
'
'        operator = Mid$(line, 4, 1) '0=none, 1=add, 2=multiply, 3=substract?
'        IsFromPointer = Mid$(line, 6, 1) '0=absolute, 1=pointer
'
'        incr_address_by = 0
'        incr_address_by = CLng(Ox & hexdata)
'
'        If IsFromPointer Then
'          Select Case operator
'           Case 0
'                   pvalue = Pointer + (pvalue + incr_address_by)
'           Case 1
'                   pvalue = Pointer - (pvalue + incr_address_by)
'           Case 2
'                   pvalue = Pointer + (pvalue * incr_address_by)
'          End Select
'        Else
'          Select Case operator
'           Case 0
'                   pvalue = (pvalue + incr_address_by)
'           Case 1
'                   pvalue = (pvalue - incr_address_by)
'           Case 2
'                   pvalue = (pvalue * incr_address_by)
'          End Select
'        End If
'
'        Select Case Mid$(line, 8, 1)
'         Case 0
'           Pointer = pvalue
'         Case 1
'           Pointer = Pointer + pvalue
'           pvalue = 0
'        End Select
'
'        GoTo nextcommand
'
'      '-- 6T1ZZZZZ XXXXXXXX  <- Pointer Code: Set Pointer operation XXXXXXXXX from pointer + ZZZZZ
'      ElseIf isactive And line Like "6[0-2]2[0-2]0[0-2]0[0-2] [0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f]" Then
'        hexdata = Right$(line, 8)
'
'        'T = Bit Size
'        Select Case Mid$(line, 2, 1)
'         Case 0: datasize = 1 '8 bit
'                 hexdata = Right$(hexdata, 2)
'
'         Case 1: datasize = 2 '16 bit
'                 hexdata = Right$(hexdata, 4)
'
'         Case 2: datasize = 4 '32 bit
'        End Select
'
'        operator = Mid$(line, 4, 1) '0=none, 1=add, 2=multiply, 3=substract?
'        IsFromPointer = Mid$(line, 6, 1) '0=absolute, 1=pointer
'
'        incr_address_by = 0
'        incr_address_by = CLng(Ox & hexdata)
'
'        If IsFromPointer Then
'          Select Case operator
'           Case 0
'                   pvalue = Pointer + (pvalue + incr_address_by)
'           Case 1
'                   pvalue = Pointer - (pvalue + incr_address_by)
'           Case 2
'                   pvalue = Pointer + (pvalue * incr_address_by)
'           Case 3
'          End Select
'        Else
'          Select Case operator
'           Case 0
'                   pvalue = (pvalue + incr_address_by)
'           Case 1
'                   pvalue = (pvalue - incr_address_by)
'           Case 2
'                   pvalue = (pvalue * incr_address_by)
'           Case 3
'          End Select
'        End If
'
'        Select Case Mid$(line, 8, 1)
'         Case 0
'           Pointer = pvalue
'         Case 1
'           Pointer = Pointer + pvalue
'           pvalue = 0
'        End Select
'
'        GoTo nextcommand
'
'      '-- 6T4ZZZZZ XXXXXXXX  <- Pointer Code: Write value XXXXXXXXX from pointer + ZZZZZ
'      ElseIf isactive And line Like "6[0-2]4[0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f] [0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f]" Then
'        hexdata = Right$(line, 8)
'
'        'T = Bit Size
'        Select Case Mid$(line, 2, 1)
'         Case 0: datasize = 1 '8 bit
'                 hexdata = Right$(hexdata, 2)
'
'         Case 1: datasize = 2 '16 bit
'                 hexdata = Right$(hexdata, 4)
'
'         Case 2: datasize = 4 '32 bit
'        End Select
'
'        operator = Mid$(line, 4, 1) '0=none, 1=add, 2=multiply, 3=substract?
'        IsFromPointer = Mid$(line, 6, 1) '0=absolute, 1=pointer
'
'        incr_address_by = -1
'
'        If IsFromPointer Then
'          Select Case operator
'           Case 0: incr_address_by = pvalue
'           Case 1: incr_address_by = Pointer + pvalue
'          End Select
'        Else
'          Select Case operator
'           Case 0: incr_address_by = Pointer
'           Case 1: incr_address_by = Pointer + pvalue
'          End Select
'        End If
'
'        If incr_address_by <= -1 Then GoTo nextcommand
'
'        hexaddress = Hex$(incr_address_by)
'        line = "0x" & hexaddress & " " & hexdata
      
      '-- 90000000 XXXXXXXX  <- Move pointer to offset in address XXXXXXXXX
      ElseIf isactive And line Like "90000000 [0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f]" Then
        'Move pointer to offset in address XXXXXXXXX (CONFIRMED CODE)
        '90000000 XXXXXXXX
        If fo Then Close #fo: fo = 0

        ltoaddress = Ox & Right$(line, 8) + 1

        fo = FreeFile
        Open Files(1) For Binary Shared As #fo
        databin = AllocString(4)
        Get #fo, ltoaddress, databin
        Close #fo
        fo = 0

        hexdata = BinToHex(databin)
        Pointer = CLng(Ox & hexdata)
        NotFound = False
        GoTo nextcommand
      
      '-- 92000000 XXXXXXXX  <- Step Forward Code
      ElseIf isactive And line Like "92000000 [0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f]" Then
        'Step Forward Code (CONFIRMED CODE)
        '92000000 XXXXXXXX
        hexdata = Mid$(line, 10, 8)
        incr_address_by = 0
        incr_address_by = CLng(Ox & hexdata)
        Pointer = Pointer + incr_address_by
        NotFound = False
      
      '-- 93000000 XXXXXXXX  <- Step Back Code
      ElseIf isactive And line Like "93000000 [0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f]" Then
        'Step Back Code (CONFIRMED CODE)
        '93000000 XXXXXXXX
        hexdata = Mid$(line, 10, 8)
        incr_address_by = 0
        incr_address_by = CLng(Ox & hexdata)
        If Pointer >= incr_address_by Then
          Pointer = Pointer - incr_address_by
          NotFound = False
        Else
          Pointer = 0
          NotFound = True
        End If
      
      '-- 94000000 XXXXXXXX  <- Step Back From End of File Code
      ElseIf isactive And line Like "94000000 [0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f]" Then
        'Step Back From End of File Code (CONFIRMED CODE)
        '94000000 XXXXXXXX
        If fo Then Close #fo: fo = 0
        If Files.Count >= 1 Then
          FileSize = FileLen(Files(1))
          Pointer = FileSize - 1
          hexdata = Mid$(line, 10, 8)
          incr_address_by = 0
          incr_address_by = CLng(Ox & hexdata)
          Pointer = Pointer - incr_address_by
          NotFound = False
        End If

      ElseIf isactive And line Like "[aA][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f] 0000[0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f]" Then
        'Multi-write
        'Axxxxxxx 0000yyyy  (xxxxxxxx = address, yyyy = size)
        'zzzzzzzz zzzzzzzz  <-data to write at address
        hexaddress = Mid$(line, 2, 7)
        datasize = CLng(Ox & Mid$(line, 14, 4))
        ntimes = 0
        databin = ""
        hexdata = ""

        line = ""

        Do Until datasize <= 0 Or EOF(fh)
          If Len(line) = 0 Then
            Line Input #fh, line
            line = modVB6.Replace(line, " ")
          End If

          If datasize >= 4 Then
            hexdata = hexdata & Mid$(line, 1, 8)
          ElseIf datasize = 3 Then
            'hexdata = hexdata & Mid$(line, 3, 6)
            hexdata = hexdata & Mid$(line, 1, 6)
          ElseIf datasize = 2 Then
            'hexdata = hexdata & Mid$(line, 5, 4)
            hexdata = hexdata & Mid$(line, 1, 4)
          ElseIf datasize = 1 Then
            'hexdata = hexdata & Mid$(line, 7, 2)
            hexdata = hexdata & Mid$(line, 1, 2)
          End If

          datasize = datasize - 4
          line = Mid$(line, 9)
        Loop
        
        line = "0x" & hexaddress & " " & hexdata
      
      '-- Search Code
      ElseIf isactive And line Like "8[0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f] [0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f]" Then
        'Search Code
        '8ZZZXXXX YYYYYYYY  ZZZ=times to search  XXXX=size of search string  YYYY=search string
        'YYYYYYYY YYYYYYYY

        ntimes = CLng(Ox & Mid$(line, 2, 3))
        datasize = CLng(Ox & Mid$(line, 5, 4))
        hexdata = Mid$(line, 10, 8)
        searchbin = ""
        Pointer = 0 'start search from offset 0x0000000
        NotFound = True

        Do Until datasize <= 0 Or EOF(fh)
          If datasize >= 4 Then
            For i = 1 To 8 Step 2
              searchbin = searchbin & Chr$(Ox & Mid$(hexdata, i, 2))
            Next
          ElseIf datasize = 3 Then
            For i = 1 To 6 Step 2
              searchbin = searchbin & Chr$(Ox & Mid$(hexdata, i, 2))
            Next
          ElseIf datasize = 2 Then
            For i = 1 To 4 Step 2
              searchbin = searchbin & Chr$(Ox & Mid$(hexdata, i, 2))
            Next
          ElseIf datasize = 1 Then
            For i = 1 To 2 Step 2
              searchbin = searchbin & Chr$(Ox & Mid$(hexdata, i, 2))
            Next
          End If

          datasize = datasize - 4
          hexdata = Mid$(hexdata, 9)

          If Len(hexdata) Then
            'continue
          ElseIf datasize > 0 Then
            line = ""
            Line Input #fh, line
            line = modVB6.Replace(line, " ")

            If line Like "[0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f]" Then
              hexdata = line
            Else
              Exit Do
            End If
          End If
        Loop

        If Len(searchbin) Then
          For f = 1 To Files.Count
            If FileExists(Files(f)) Then
              If fo = 0 Then
                fo = FreeFile
                Open Files(f) For Binary Shared As #fo
                databin = AllocString(LOF(fo))
                Get #fo, , databin
              End If

              For i = 1 To ntimes
                Pointer = InStr(Pointer + 1, databin, searchbin, vbBinaryCompare)
                If Pointer = 0 Then Exit For
              Next

              If Files.Count = 1 Then
                'keep file open
              Else
                databin = ""
                Close #fo
                fo = 0
              End If

              If Pointer Then
                Pointer = Pointer - 1
                NotFound = False
                Exit For
              End If
            End If
          Next
        End If
      End If
      '----------------------------------

parseline:
      If Len(line) = 0 Or lc < FromLine Then
        'ignore line
      ElseIf line Like ";*" Then
        'ignore line
      ElseIf lline Like "path:*" Then
        line = "\" & NormalizePath(Trim$(Mid$(line, 6)))
        lline = LCase$(line)

        If LCase$(Right$(NormalizePath(savepath), Len(line))) Like lline Then Exit Do
        Pointer = 0
      ElseIf lline Like "carry(*)" Then
      
      ElseIf isactive And (lline Like "set *:*") Then
        'address
        Address = -1 'exception address
        pos = InStr(line, ":") + 1
        hexaddress = Trim$(Mid$(line, pos))
        parameter = LCase$(hexaddress)
        varname = ""

        If parameter Like "lastbyte*" Then
          If fo Then Close #fo: fo = 0
          If Files.Count >= 1 Then
            FileSize = FileLen(Files(1))
            Pointer = FileSize - 1
            hexaddress = Trim$(Mid$(hexaddress, 9))
            If hexaddress = "" Then hexaddress = "+0"
          End If
        ElseIf parameter Like "eof*" Then
          If fo Then Close #fo: fo = 0
          If Files.Count >= 1 Then
            FileSize = FileLen(Files(1))
            Pointer = FileSize - 1
            hexaddress = Trim$(Mid$(hexaddress, 4))
            If hexaddress = "" Then hexaddress = "+0"
          End If
        ElseIf parameter Like VariableMask & "*" Then
          pos = InStr(hexaddress, "]")
          varname = Mid$(hexaddress, 2, pos - 2)
          hexaddress = Trim$(Mid$(hexaddress, pos + 1))
          If hexaddress Like "+*" Then
            hexaddress = ParseExpression(vars, hexaddress)
            If Left$(hexaddress, 1) <> "+" Then hexaddress = "+" & hexaddress
          ElseIf hexaddress Like "-*" Then
            hexaddress = ParseExpression(vars, hexaddress)
            If Left$(hexaddress, 1) <> "+" Then hexaddress = "+" & hexaddress
          ElseIf hexaddress Like "&*" Then
            'hexaddress = vars(varname) & ParseExpression(vars, hexaddress)
          End If
        ElseIf parameter Like """*""*" Then
          pos = InStr(line, ":")
          varname = Trim$(Mid$(line, 5, pos - 5))
          If varname Like VariableMask Then
            varname = RemoveBrackets(varname)
          End If

          hexdata = ParseExpression(vars, hexaddress)
          
          SetVariable vars, varname, hexdata, Pointer
          GoTo nextcommand
        ElseIf IsIn(parameter, "xor:*", "not *", "md5*", "crc*", "eachecksum", "adler16", "adler32", "sha1*", "sha256", "sha384", "sha512", "hmac*", "md4*", "md2*", "userid*", "titleid*", "psid*", "account*", "profile*", "read(*,*)*", "xor(*,*,*)*", "add(*,*)", "wadd(*,*)", "[dq]wadd(*,*)", "sub(*,*)", "wsub(*,*)", "[dq]wsub(*,*)", "repeat(*,*)*", "mid(*,*)", "left(*,*)", "right(*,*)") Then
          pos = InStr(line, ":")
          varname = Trim$(Mid$(line, 5, pos - 5))
          If varname Like VariableMask Then
            varname = RemoveBrackets(varname)
          ElseIf varname = "pointer" Then
            'continue
          Else
            GoTo nextcommand
          End If

          If fo Then Close #fo: fo = 0

          parameter = LCase$(hexaddress)
          If parameter Like "xor:*" Then
            parameter = Trim$(Mid$(parameter, 5))
            parameter = modVB6.Replace$(parameter, "0x")
            hexdata = vars(varname)
            hexdata = Right$("00000000" & Hex$(CLng(Ox & hexdata) Xor CLng(Ox & parameter)), Len(hexdata))
          ElseIf parameter Like "not *" Then
            parameter = Trim$(Mid$(parameter, 5))
            parameter = modVB6.Replace$(parameter, "0x")
            parameter = ConvertVariableToValue(vars, parameter)
            hexdata = Right$("00000000" & Hex$(Not CLng(Ox & parameter)), 8)
          ElseIf parameter Like "md5_xor*" Then
            md5.Algorythm = ALG_SID_MD5
            hexdata = md5.HashFile(Files(1), StartPadding:=StartRange, Padding:=endrange)
            c = Val("&H" & Mid$(hexdata, 1, 8) & "&") Xor _
                Val("&H" & Mid$(hexdata, 9, 8) & "&") Xor _
                Val("&H" & Mid$(hexdata, 17, 8) & "&") Xor _
                Val("&H" & Mid$(hexdata, 25, 8) & "&")

            hexdata = Right$("0000000" & Hex$(c), 8)
            hexdata = Mid$(hexdata, 7, 2) & Mid$(hexdata, 5, 2) & Mid$(hexdata, 3, 2) & Mid$(hexdata, 1, 2)
          ElseIf parameter Like "md5*" Then
            md5.Algorythm = ALG_SID_MD5
            hexdata = md5.HashFile(Files(1), StartPadding:=StartRange, Padding:=endrange)
          ElseIf parameter Like "sha1*" Then
            md5.Algorythm = ALG_SID_SHA1
            hexdata = md5.HashFile(Files(1), StartPadding:=StartRange, Padding:=endrange)
          ElseIf IsIn(parameter, "sha256", "sha384", "sha512") Then
            c = FreeFile
            Open Files(1) For Binary As #c
            databin = AllocString(LOF(c) - StartRange - endrange)
            Get #c, StartRange + 1, databin
            Close #c
            
            If parameter = "sha384" Then
              hexdata = Crypto_Hash(databin, Algorithm:=HASH_SHA384)
            ElseIf parameter = "sha512" Then
              hexdata = Crypto_Hash(databin, Algorithm:=HASH_SHA512)
            Else
              hexdata = Crypto_Hash(databin, Algorithm:=HASH_SHA256)
            End If
          ElseIf parameter Like "hmac_sha1(*)*" Then
            pos = Len(hexaddress)
            fromaddress = ""
            fromaddress = HexToBin(ParseExpression(vars, Mid$(hexaddress, 11, pos - 10)))
            If IsTLOU And Len(fromaddress) <= 1 Then
              fromaddress = TLOU_HMAC_KEY
            End If

            c = FreeFile
            Open Files(1) For Binary As #c
            databin = AllocString(LOF(c) - StartRange - endrange)
            Get #c, StartRange + 1, databin
            Close #c
            
            hexdata = Crypto_Hash(databin, fromaddress, HMAC_SHA1)
          ElseIf parameter Like "md4*" Then
            md5.Algorythm = ALG_SID_MD4
            hexdata = md5.HashFile(Files(1), StartPadding:=StartRange, Padding:=endrange)
          ElseIf parameter Like "md2*" Then
            md5.Algorythm = ALG_SID_MD2
            hexdata = md5.HashFile(Files(1), StartPadding:=StartRange, Padding:=endrange)
          ElseIf parameter Like "crc16*" Then
            crc.Algorithm = CRC16
            hexdata = crc.HashFile(Files(1), StartPadding:=StartRange, Padding:=endrange)
          ElseIf parameter Like "crc32big" Then
            crc.Algorithm = CRC32_BigEndian
            hexdata = crc.HashFile(Files(1), StartPadding:=StartRange, Padding:=endrange)
          ElseIf parameter Like "crc64_iso" Then
            crc.Algorithm = CRC64_ISO
            hexdata = crc.HashFile(Files(1), StartPadding:=StartRange, Padding:=endrange)
          ElseIf parameter Like "crc64_ecma" Then
            crc.Algorithm = CRC64_ECMA182
            hexdata = crc.HashFile(Files(1), StartPadding:=StartRange, Padding:=endrange)
            
            If IsFuse Then
              crc.Clear
              hexdata = crc.AddString(HexToBin("3141281814285714" & hexdata))
            End If

          ElseIf parameter Like "crc32big*" Then 'crc32big:<initial value>:xor:<xor value>
            crc.Algorithm = CRC32_BigEndian
            If parameter Like "*:xor:*" Then
              pos = InStr(1, parameter, ":xor:", vbTextCompare)
              databin = "FFFFFFFF"
              databin = Trim$(Mid$(parameter, 10, pos - 10))
              databin = ConvertVariableToValue(vars, databin)

              crc.Clear CLng(modVB6.Replace(Ox & databin, "0x"))
              hexdata = "00000000"
              hexdata = Trim$(Mid$(parameter, pos + 5))
              hexdata = ConvertVariableToValue(vars, hexdata)
              c = CLng(modVB6.Replace(Ox & hexdata, "0x"))
            Else
              hexdata = Trim$(Mid$(parameter, 10))
              hexdata = ConvertVariableToValue(vars, hexdata)
              
              crc.Clear CLng(Ox & modVB6.Replace(hexdata, "0x"))
              c = 0
            End If
            hexdata = crc.HashFile(Files(1), StartPadding:=StartRange, Padding:=endrange)
            If c Then
              hexdata = Right$(String$(Len(hexdata), "0") & Hex$(c Xor CLng(Ox & hexdata)), Len(hexdata))
            End If
          ElseIf parameter Like "crc32little" Then
            crc.Algorithm = CRC32_LittleEndian
            hexdata = crc.HashFile(Files(1), StartPadding:=StartRange, Padding:=endrange)
          ElseIf parameter Like "crc32little*" Then  'crc32little:<initial value>:xor:<xor value>
            crc.Algorithm = CRC32_LittleEndian
            If parameter Like "*:xor:*" Then
              pos = InStr(1, parameter, ":xor:", vbTextCompare)
              databin = "FFFFFFFF"
              databin = Trim$(Mid$(parameter, 13, pos - 13))
              databin = ConvertVariableToValue(vars, databin)
              
              crc.Clear CLng(modVB6.Replace(Ox & databin, "0x"))
              hexdata = "00000000"
              hexdata = Trim$(Mid$(parameter, pos + 5))
              hexdata = ConvertVariableToValue(vars, hexdata)
              
              c = CLng(modVB6.Replace(Ox & hexdata, "0x"))
            Else
              hexdata = Trim$(Mid$(parameter, 13))
              hexdata = ConvertVariableToValue(vars, hexdata)
              
              crc.Clear CLng(Ox & modVB6.Replace(hexdata, "0x"))
              c = 0
            End If
            hexdata = crc.HashFile(Files(1), StartPadding:=StartRange, Padding:=endrange)
            If c Then
              hexdata = Right$(String$(Len(hexdata), "0") & Hex$(c Xor CLng(Ox & hexdata)), Len(hexdata))
            End If
          ElseIf parameter Like "crc32" Then
            crc.Algorithm = CRC32
            hexdata = crc.HashFile(Files(1), StartPadding:=StartRange, Padding:=endrange)
          ElseIf parameter Like "crc32*" Then 'crc32:<initial value>
            crc.Algorithm = CRC32
            If parameter Like "*:xor:*" Then
              pos = InStr(1, parameter, ":xor:", vbTextCompare)
              databin = "FFFFFFFF"
              databin = Trim$(Mid$(parameter, 7, pos - 7))
              databin = ConvertVariableToValue(vars, databin)
              
              crc.Clear CLng(modVB6.Replace(Ox & databin, "0x"))
              hexdata = "00000000"
              hexdata = Trim$(Mid$(parameter, pos + 5))
              hexdata = ConvertVariableToValue(vars, hexdata)
              
              c = CLng(modVB6.Replace(Ox & hexdata, "0x"))
            Else
              hexdata = Trim$(Mid$(parameter, 7))
              hexdata = ConvertVariableToValue(vars, hexdata)
              
              crc.Clear CLng(Ox & modVB6.Replace(hexdata, "0x"))
              c = 0
            End If
            hexdata = crc.HashFile(Files(1), StartPadding:=StartRange, Padding:=endrange)
            If c Then
              hexdata = Right$(String$(Len(hexdata), "0") & Hex$(c Xor CLng(Ox & hexdata)), Len(hexdata))
            End If
          ElseIf parameter Like "crc" Then
            crc.Algorithm = CustomCRC
            hexdata = crc.HashFile(Files(1), StartPadding:=StartRange, Padding:=endrange)
          ElseIf parameter Like "crc:*" Then 'crc:<initial value>:xor:<xor value>
            crc.Algorithm = CustomCRC
            If parameter Like "*:xor:*" Then
              pos = InStr(1, parameter, ":xor:", vbTextCompare)
              hexdata = "FFFFFFFF"
              hexdata = Mid$(parameter, 5, pos - 5)
              hexdata = ConvertVariableToValue(vars, hexdata)
              
              crc.CustomCRC_InitialValue = CLng(modVB6.Replace(Ox & hexdata, "0x"))
              hexdata = Mid$(parameter, pos + 5)
              hexdata = ConvertVariableToValue(vars, hexdata)
              
              crc.CustomCRC_XOR = CLng(modVB6.Replace(Ox & hexdata, "0x"))
            Else
              hexdata = Trim$(Mid$(parameter, 5))
              hexdata = ConvertVariableToValue(vars, hexdata)
              
              crc.Clear CLng(Ox & modVB6.Replace(hexdata, "0x"))
            End If
            hexdata = crc.HashFile(Files(1), StartPadding:=StartRange, Padding:=endrange)
          ElseIf parameter Like "eachecksum" Then
            crc.Algorithm = EA_Algorithm
            hexdata = crc.HashFile(Files(1), StartPadding:=StartRange, Padding:=endrange)
          ElseIf parameter Like "adler32" Then
            crc.Algorithm = Adler32
            hexdata = crc.HashFile(Files(1), StartPadding:=StartRange, Padding:=endrange)
          ElseIf parameter Like "adler16" Then
            crc.Algorithm = Adler16
            hexdata = crc.HashFile(Files(1), StartPadding:=StartRange, Padding:=endrange)
'#If False Then
          ElseIf parameter Like "psid*" Then
            hexdata = Mid$(formMain.lstPath.SelectedItem.SubItems(colPARMS), 60, 35)
            hexdata = modVB6.Replace(hexdata, "-")
          ElseIf parameter Like "titleid*" Then
            hexdata = formMain.lstPath.SelectedItem.SubItems(colTitleID)
            hexdata = BinToHex(hexdata)
          ElseIf parameter Like "userid*" Then
            f = CLng(Mid$(formMain.lstPath.SelectedItem.SubItems(colPARMS), 6, 8))
            hexdata = Chr$(f And &HFF&) & Chr$((f And &HFF00&) \ &H100&) & Chr$((f And &HFF0000) \ &H10000) & Chr$((f And &HFF000000) \ &H1000000)
          ElseIf parameter Like "account*" Then
            hexdata = Trim$(Mid$(formMain.lstPath.SelectedItem.SubItems(colPARMS), 31, 16))
          ElseIf parameter Like "profile*" Then
            hexdata = Mid$(formMain.lstPath.SelectedItem.SubItems(colPARMS), 31, 16)
            hexdata = BinToHex(hexdata)
'#End If
          ElseIf IsIn(parameter, "add(*,*)", "wadd(*,*)", "[dq]wadd(*,*)", "sub(*,*)", "wsub(*,*)", "[dq]wsub(*,*)") Then
            If Files.Count >= 1 Then
              pos = InStr(parameter, ",")
              woperator = 0
              
              If parameter Like "add(*,*)" Then
                datasize = 1
                fromaddress = Trim$(Mid$(parameter, 5, pos - 5))
                woperator = 1
              ElseIf parameter Like "wadd(*,*)" Then
                datasize = 2
                fromaddress = Trim$(Mid$(parameter, 6, pos - 6))
                woperator = 2
              ElseIf parameter Like "dwadd(*,*)" Then
                datasize = 4
                fromaddress = Trim$(Mid$(parameter, 7, pos - 7))
                woperator = 4
              ElseIf parameter Like "qwadd(*,*)" Then
                datasize = 8
                fromaddress = Trim$(Mid$(parameter, 7, pos - 7))
                woperator = 8
              ElseIf parameter Like "sub(*,*)" Then
                datasize = 1
                fromaddress = Trim$(Mid$(parameter, 5, pos - 5))
                woperator = -1 'substract
              ElseIf parameter Like "wsub(*,*)" Then
                datasize = 2
                fromaddress = Trim$(Mid$(parameter, 6, pos - 6))
                woperator = -2 'substract
              ElseIf parameter Like "dwsub(*,*)" Then
                datasize = 4
                fromaddress = Trim$(Mid$(parameter, 7, pos - 7))
                woperator = -4 'substract
              ElseIf parameter Like "qwsub(*,*)" Then
                datasize = 8
                fromaddress = Trim$(Mid$(parameter, 7, pos - 7))
                woperator = -8 'substract
              End If

              parameter = Mid$(parameter, pos + 1)
              
              pos = InStr(parameter, ")")
              toaddress = Trim$(Left$(parameter, pos - 1))
              
              fromaddress = ParseAddress(vars, fromaddress, Files, Pointer)
              Address = 0
              Address = CLng(Ox & fromaddress) + 1

              toaddress = ParseAddress(vars, toaddress, Files, Pointer)
              ltoaddress = 0
              ltoaddress = CLng(Ox & toaddress) + 1

              sum = 0
              sum = CLng(Ox & vars(varname))
              
              databin = AllocString(datasize)

              If datasize < 4 Then
                ReDim bytes(3) As Byte
              Else
                ReDim bytes(datasize - 1) As Byte
              End If

              If fo Then Close #fo
              fo = FreeFile
              Open Files(1) For Binary Shared As #fo
              
              Get #fo, Address - 1, bytes(0)

              If datasize = 1 Then
                ReDim bytes(ltoaddress - Address) As Byte
                Get #fo, , bytes
                
                For pos = 0 To UBound(bytes)
                  c = 0 + bytes(pos) '//fixes bug when compiled with Native code
                  sum = sum + c
                
                  If CarryBytes <= 0 Then
                    'do not carry bytes
                  Else
                    'http://en.wikipedia.org/wiki/Fletcher's_checksum
                    Do
                      hexdata = Hex$(sum)
                      If Len(hexdata) > 2 * CarryBytes Then
                        sum = Deca(Right$(hexdata, 2 * CarryBytes)) + Deca(Left$(hexdata, Len(hexdata) - 2 * CarryBytes))
                      Else
                        Exit Do
                      End If
                    Loop
                  End If
                Next
              ElseIf datasize = 2 Then
                ReDim bytes(ltoaddress - Address) As Byte
                Get #fo, , bytes
                
                For pos = 0 To UBound(bytes) Step 2
                  c = 0& + 256& * bytes(pos) + bytes(pos + 1)
                  sum = sum + c
      
                  If CarryBytes <= 0 Then
                    'do not carry bytes
                  Else
                    'http://en.wikipedia.org/wiki/Fletcher's_checksum
                    Do
                      hexdata = Hex$(sum)
                      If Len(hexdata) > 2 * CarryBytes Then
                        sum = Deca(Right$(hexdata, 2 * CarryBytes)) + Deca(Left$(hexdata, Len(hexdata) - 2 * CarryBytes))
                      Else
                        Exit Do
                      End If
                    Loop
                  End If
                Next
              ElseIf datasize = 4 Then
                ReDim bytes(ltoaddress - Address) As Byte
                Get #fo, , bytes
                
                For pos = 0 To UBound(bytes) Step 4
                  sum = sum + 16777216@ * bytes(pos) + 65536@ * bytes(pos + 1) + 256@ * bytes(pos + 2) + 1@ * bytes(pos + 3)
      
                  If CarryBytes <= 0 Then
                    'do not carry bytes
                  Else
                    'http://en.wikipedia.org/wiki/Fletcher's_checksum
                    Do
                      hexdata = Hex$(sum)
                      If Len(hexdata) > 2 * CarryBytes Then
                        sum = Deca(Right$(hexdata, 2 * CarryBytes)) + Deca(Left$(hexdata, Len(hexdata) - 2 * CarryBytes))
                      Else
                        Exit Do
                      End If
                    Loop
                  End If
                Next
              ElseIf datasize = 8 Then
                ReDim bytes(ltoaddress - Address) As Byte
                Get #fo, , bytes
                
                For pos = 0 To UBound(bytes) Step 8
                  sum = sum + 16777216@ * bytes(pos + 4) + 65536@ * bytes(pos + 5) + 256@ * bytes(pos + 6) + 1@ * bytes(pos + 7)
          
                  If CarryBytes <= 0 Then
                    'do not carry bytes
                  Else
                    'http://en.wikipedia.org/wiki/Fletcher's_checksum
                    Do
                      hexdata = Hex$(sum)
                      If Len(hexdata) > 2 * CarryBytes Then
                        sum = Deca(Right$(hexdata, 2 * CarryBytes)) + Deca(Left$(hexdata, Len(hexdata) - 2 * CarryBytes))
                      Else
                        Exit Do
                      End If
                    Loop
                  End If
                Next

              ElseIf datasize = -1 Then
                ReDim bytes(ltoaddress - Address) As Byte
                Get #fo, , bytes
                
                For pos = 0 To UBound(bytes)
                  c = 0 + bytes(pos) '//fixes bug when compiled with Native code
                  sum = sum - c
                
                  If CarryBytes <= 0 Then
                    'do not carry bytes
                  Else
                    'http://en.wikipedia.org/wiki/Fletcher's_checksum
                    Do
                      hexdata = Hex$(sum)
                      If Len(hexdata) > 2 * CarryBytes Then
                        sum = Deca(Right$(hexdata, 2 * CarryBytes)) + Deca(Left$(hexdata, Len(hexdata) - 2 * CarryBytes))
                      Else
                        Exit Do
                      End If
                    Loop
                  End If
                Next
              ElseIf datasize = -2 Then
                ReDim bytes(ltoaddress - Address) As Byte
                Get #fo, , bytes
                
                For pos = 0 To UBound(bytes) Step 2
                  c = 0& + 256& * bytes(pos) + bytes(pos + 1)
                  sum = sum - c
      
                  If CarryBytes <= 0 Then
                    'do not carry bytes
                  Else
                    'http://en.wikipedia.org/wiki/Fletcher's_checksum
                    Do
                      hexdata = Hex$(sum)
                      If Len(hexdata) > 2 * CarryBytes Then
                        sum = Deca(Right$(hexdata, 2 * CarryBytes)) + Deca(Left$(hexdata, Len(hexdata) - 2 * CarryBytes))
                      Else
                        Exit Do
                      End If
                    Loop
                  End If
                Next
              ElseIf datasize = -4 Then
                ReDim bytes(ltoaddress - Address) As Byte
                Get #fo, , bytes
                
                For pos = 0 To UBound(bytes) Step 4
                  sum = sum - (16777216@ * bytes(pos) + 65536@ * bytes(pos + 1) + 256@ * bytes(pos + 2) + 1@ * bytes(pos + 3))
      
                  If CarryBytes <= 0 Then
                    'do not carry bytes
                  Else
                    'http://en.wikipedia.org/wiki/Fletcher's_checksum
                    Do
                      hexdata = Hex$(sum)
                      If Len(hexdata) > 2 * CarryBytes Then
                        sum = Deca(Right$(hexdata, 2 * CarryBytes)) + Deca(Left$(hexdata, Len(hexdata) - 2 * CarryBytes))
                      Else
                        Exit Do
                      End If
                    Loop
                  End If
                Next
              ElseIf datasize = -8 Then
                ReDim bytes(ltoaddress - Address) As Byte
                Get #fo, , bytes
                
                For pos = 0 To UBound(bytes) Step 8
                  sum = sum - (16777216@ * bytes(pos + 4) + 65536@ * bytes(pos + 5) + 256@ * bytes(pos + 6) + 1@ * bytes(pos + 7))

                  If CarryBytes <= 0 Then
                    'do not carry bytes
                  Else
                    'http://en.wikipedia.org/wiki/Fletcher's_checksum
                    Do
                      hexdata = Hex$(sum)
                      If Len(hexdata) > 2 * CarryBytes Then
                        sum = Deca(Right$(hexdata, 2 * CarryBytes)) + Deca(Left$(hexdata, Len(hexdata) - 2 * CarryBytes))
                      Else
                        Exit Do
                      End If
                    Loop
                  End If
                Next
              End If
              Close #fo
              fo = 0

              If CarryBytes <= 0 Then
                hexdata = Right$("0000000" & hexa(sum), 8)
              Else
                hexdata = Right$("0000000" & hexa(sum), 2 * CarryBytes)
              End If
            Else
              hexdata = ""
            End If
          ElseIf parameter Like "repeat(*,*,*)*" Or parameter Like "repeat(*,*)*" Then 'repeat(<count>,<value>,<incr>)
            If Files.Count >= 1 Then
              pos = InStr(parameter, ",")
              fromaddress = Trim$(Mid$(parameter, 8, pos - 8))
              parameter = Mid$(parameter, pos + 1)
              
              pos = InStr(parameter, ",")
              If pos = 0 Then pos = InStr(parameter, ")")
              
              toaddress = Trim$(Left$(parameter, pos - 1))
              parameter = Mid$(parameter, pos + 1)
              
              pos = InStr(parameter, "))")
              If pos Then
                pos = pos + 1
              Else
                pos = InStr(parameter, ")")
              End If
              databin = Trim$(Left$(parameter, pos - 1))
              
              fromaddress = ParseAddress(vars, fromaddress, Files, Pointer)
              Address = 0
              Address = CLng(Ox & fromaddress)
              
              toaddress = ParseAddress(vars, toaddress, Files, Pointer)
              ltoaddress = 0
              ltoaddress = CLng(Ox & toaddress)

              databin = ParseAddress(vars, databin, Files, Pointer)
              incr_value_by = 0
              incr_value_by = CLng(Ox & databin)
             
              hexdata = ""
              
              If incr_value_by = 0 Then
                For pos = 1 To Address
                  hexdata = hexdata & toaddress
                Next
              Else
                For pos = 1 To Address
                  toaddress = Right$(String$(Len(toaddress), "0") & Hex$(ltoaddress), Len(toaddress))
                  hexdata = hexdata & toaddress
                  ltoaddress = ltoaddress + incr_value_by
                Next
              End If

              If line Like "set [[]" & operator & "x[0-9A-F]*[]]:repeat*" Then
                pos = InStr(line, "]")
                line = Mid$(line, 6, pos - 6) & " " & hexdata
                lline = LCase$(line)
                GoTo parsewrite
              End If
            Else
              hexdata = ""
            End If

          ElseIf parameter Like "xor(*,*,*)*" Then 'xor(<start>,<endrange>,<incr>)
            If Files.Count >= 1 Then
              pos = InStr(parameter, ",")
              fromaddress = Trim$(Mid$(parameter, 5, pos - 5))
              parameter = Mid$(parameter, pos + 1)
              
              pos = InStr(parameter, ",")
              toaddress = Trim$(Left$(parameter, pos - 1))
              parameter = Mid$(parameter, pos + 1)
              
              pos = InStr(parameter, ")")
              databin = Trim$(Left$(parameter, pos - 1))

              fromaddress = ParseAddress(vars, fromaddress, Files, Pointer)
              Address = 0
              Address = CLng(Ox & fromaddress) + 1
              
              toaddress = ParseAddress(vars, toaddress, Files, Pointer)
              ltoaddress = 0
              ltoaddress = CLng(Ox & toaddress) + 1

              databin = ParseAddress(vars, databin, Files, Pointer)
              datasize = 0
              datasize = CLng(Ox & databin)
              
              databin = AllocString(datasize)
             
              If datasize Then
                c = 0
                c = CLng(Ox & vars(varname))
  
                If fo Then Close #fo
                fo = FreeFile
                Open Files(1) For Binary Shared As #fo
                
                '---------
                ReDim bytes(ltoaddress - Address) As Byte
                Dim X(3) As Byte
                
                databin = Right$("0000000" & Hex$(c), 8)
                X(0) = CInt(Ox & Mid$(databin, 1, 2))
                X(1) = CInt(Ox & Mid$(databin, 3, 2))
                X(2) = CInt(Ox & Mid$(databin, 5, 2))
                X(3) = CInt(Ox & Mid$(databin, 7, 2))

                Get #fo, Address, bytes
                For pos = 0 To UBound(bytes) Step datasize
                  For i = 0 To datasize - 1
                    X(i) = X(i) Xor bytes(pos + i)
                  Next
                Next

                If datasize = 4 Then
                  c = CLng(Ox & BinToHex(Chr$(X(0)) & Chr$(X(1)) & Chr$(X(2)) & Chr$(X(3))))
                ElseIf datasize = 3 Then
                  c = CLng(Ox & BinToHex(Chr$(X(0)) & Chr$(X(1)) & Chr$(X(2))))
                ElseIf datasize = 2 Then
                  c = CLng(Ox & BinToHex(Chr$(X(0)) & Chr$(X(1))))
                ElseIf datasize = 1 Then
                  c = X(0)
                End If
                '---------

                'For pos = Address To ltoaddress Step datasize
                '  Get #fo, pos, databin
                '  c = c Xor CLng(Ox & BinToHex(databin))
                'Next
                
                Close #fo
                fo = 0
              End If
              hexdata = Right$("0000000" & Hex$(c), datasize * 2)
            Else
              hexdata = ""
            End If
          
          ElseIf IsIn(parameter, "mid([[]*[]],*,*)", "mid([[]*[]],*)", "left([[]*[]],*)", "right([[]*[]],*)") Then
            hexdata = ""
            hexdata = ParseExpression(vars, parameter)
          ElseIf parameter Like "read(*,*)*" Then
            If Files.Count >= 1 Then
              pos = InStr(parameter, ",")
              fromaddress = Trim$(Mid$(parameter, 6, pos - 6))
              parameter = Mid$(parameter, pos + 1)
              pos = InStr(parameter, ")")
              
              databin = Trim$(Mid$(parameter, 1, pos - 1))
              If databin Like "(*" Then databin = databin & ")"

              fromaddress = ParseAddress(vars, fromaddress, Files, Pointer)
              ltoaddress = 0
              ltoaddress = CLng(Ox & fromaddress) + 1
             
              databin = ParseAddress(vars, databin, Files, Pointer)
              datasize = 0
              datasize = CLng(Ox & databin)
             
              fo = FreeFile
              Open Files(1) For Binary Shared As #fo
              databin = AllocString(datasize)
              Get #fo, ltoaddress, databin
              Close #fo
              fo = 0
            
              hexdata = BinToHex(databin)
            Else
              hexdata = ""
            End If
          Else
            hexdata = BinToHex(parameter)
          End If
          
          SetVariable vars, varname, hexdata, Pointer
          GoTo nextcommand
        ElseIf IsIn(line, "set crc_*:*") Then
          hexaddress = &H1000&
        ElseIf IsIn(line, "set range:*,*", "set crc_*:*", "set md5:*", "set md2:*", "set sha1:*", "set crc32:*", "set crc32big:*", "set crc32little:*", "set crc16:*", "set adler16:*", "set adler32:*", "set psid:*", "set userid:*", "set titleid:*", "set *account*:*", "set *profile*:*", "set ""*"":*", "set pointer:*") Then
        ElseIf IsNumeric(hexaddress) Or hexaddress Like "0x[0-9A-Fa-f]*" Then
          pos = InStr(line, ":")
          varname = Trim$(Mid$(line, 5, pos - 5))
          If varname Like VariableMask Then
            varname = RemoveBrackets(varname)
          End If

          hexdata = ParseAddress(vars, hexaddress, Files, Pointer)

          SetVariable vars, varname, hexdata, Pointer
          GoTo nextcommand
        End If

        c = Pointer
        c64 = 0

        Select Case Left$(hexaddress, 1)
         Case "+": operator = 1: hexaddress = Trim$(Mid$(hexaddress, 2))
                   If Len(varname) Then
                    If Len(vars(varname)) <= 8 Then
                      c = CLng(Ox & vars(varname))
                    Else
                      c64 = Val64(vars(varname))
                    End If
                   End If
         Case "-": operator = -1: hexaddress = Trim$(Mid$(hexaddress, 2))
                   If Len(varname) Then
                    If Len(vars(varname)) <= 8 Then
                      c = CLng(Ox & vars(varname))
                    Else
                      c64 = Val64(vars(varname))
                    End If
                   End If
         Case Else: operator = 0
                    If Len(varname) Then
                      hexaddress = vars(varname)
                    End If
        End Select

        hexaddress = ParseAddress(vars, hexaddress, Files, Pointer)
        Address = 0
        Address = ParseNumber(vars, Ox & hexaddress)

        Select Case operator
         Case 1:  Address = c + Address: Address64 = c64 + Address
         Case -1: If c >= Address Then Address = c - Address Else Address = 0
        End Select

        If Address >= 0 Or IsIn(lline, "set crc*", "set [[]*[]]:*", "set *account*:", "set *profile*:*", "set *userid*:*", "set *titleid*:*", "set psid:*", "set ""*"":*") Then
          For f = 1 To Files.Count
            If FileExists(Files(f)) Then
              If fo Then Close #fo: fo = 0

              If lline Like "set crc_bandwidth:*" Then
                databin = Trim$(Mid(line, 19))
                If databin = "32" Then
                ElseIf databin = "24" Then
                ElseIf databin = "16" Then
                Else
                  databin = ParseHexNumber(vars, databin)
                End If

                i = ValU(databin)

                If i >= 8 And i <= 32 Then
                  crc.Algorithm = CustomCRC
                  crc.CustomCRC_BitsOrder = i
                Else
                  crc.Algorithm = CustomCRC
                  crc.CustomCRC_BitsOrder = bits32
                End If
                GoTo nextcommand
              ElseIf lline Like "set crc_polynomial:*" Then
                databin = Trim$(Mid(line, 20))
                databin = ParseHexNumber(vars, databin)
                
                crc.Algorithm = CustomCRC
                crc.CustomCRC_Polynomial = CLng(databin)
                GoTo nextcommand
              ElseIf lline Like "set crc_reflection:*" Then
                databin = Trim$(Mid(line, 20))
                pos = InStr(databin, ":")

                If pos Then
                  hexdata = Trim$(Mid$(databin, pos + 1))
                  hexdata = ParseHexNumber(vars, hexdata)

                  databin = Trim$(Left(databin, pos - 1))
                  databin = ParseHexNumber(vars, databin)
                
                  crc.Algorithm = CustomCRC
                  crc.CustomCRC_reflect_input = CLng(databin)
                  crc.CustomCRC_reflect_output = CLng(hexdata)
                Else
                  databin = Trim$(Left(databin, pos - 1))
                  databin = ParseHexNumber(vars, databin)
                
                  crc.Algorithm = CustomCRC
                  crc.CustomCRC_reflect_input = CLng(databin)
                  crc.CustomCRC_reflect_output = False
                End If
                GoTo nextcommand
              ElseIf lline Like "set crc_reflection_input:*" Then
                databin = Trim$(Mid(line, 26))
                databin = ParseHexNumber(vars, databin)
                
                crc.Algorithm = CustomCRC
                crc.CustomCRC_reflect_input = CLng(databin)
                
                GoTo nextcommand
              ElseIf lline Like "set crc_reflection_output:*" Then
                databin = Trim$(Mid(line, 27))
                databin = ParseHexNumber(vars, databin)
                
                crc.Algorithm = CustomCRC
                crc.CustomCRC_reflect_output = CLng(databin)

                GoTo nextcommand
              ElseIf lline Like "set crc_initial_value:*" Then
                databin = Trim$(Mid(line, 23))
                databin = ParseHexNumber(vars, databin)
                
                crc.Algorithm = CustomCRC
                crc.CustomCRC_InitialValue = CLng(databin)

                GoTo nextcommand
              ElseIf lline Like "set crc_output_xor:*" Then
                databin = Trim$(Mid(line, 20))
                databin = ParseHexNumber(vars, databin)
                
                crc.Algorithm = CustomCRC
                crc.CustomCRC_XOR = CLng(databin)

                GoTo nextcommand
              ElseIf lline Like "set range:*,*" Then
                parameter = line
                pos = InStr(parameter, ",")
                
                fromaddress = Trim$(Mid$(parameter, 11, pos - 11))
                parameter = Mid$(parameter, pos + 1)
                
                databin = Trim$(parameter)
                If databin Like "(*" Then databin = databin & ")"
  
                fromaddress = ParseAddress(vars, fromaddress, Files, Pointer)
                StartRange = 0
                StartRange = CLng(Ox & fromaddress)

                If (LCase$(databin) Like "pointer*" And prevline Like "set pointer:eof-*") Or LCase$(databin) Like "eof*" Then
                  databin = ParseAddress(vars, databin, Files, Pointer)
                  endrange = 0
                  endrange = CLng(Ox & databin) - 1 '<-- adjust old bug in set pointer:eof-3
                Else
                  databin = ParseAddress(vars, databin, Files, Pointer)
                  endrange = 0
                  endrange = CLng(Ox & databin)
                End If

                If endrange > 0 Then
                  endrange = (FileLen(Files(f)) - 1) - endrange
                  If endrange < 0 Then endrange = 0
                Else
                  endrange = Abs(endrange)
                End If

                GoTo nextcommand
              ElseIf lline Like "set md5:*" Then
                md5.Algorythm = ALG_SID_MD5
                If Address <= StartRange Then
                  If StartRange <= 16 Then StartRange = 16
                  hexdata = md5.HashFile(Files(f), StartPadding:=StartRange, Padding:=endrange)
                ElseIf Len(varname) Then
                  hexdata = md5.HashFile(Files(f))
                  SetVariable vars, varname, hexdata, Pointer
                  GoTo nextcommand
                Else
                  hexdata = md5.HashFile(Files(f), StartPadding:=StartRange, Padding:=16)
                End If
              ElseIf lline Like "set sha1:*" Then
                md5.Algorythm = ALG_SID_SHA1
                If Address <= StartRange Then
                  If StartRange <= 20 Then StartRange = 20
                  hexdata = md5.HashFile(Files(f), StartPadding:=StartRange, Padding:=endrange)
                ElseIf Len(varname) Then
                  hexdata = md5.HashFile(Files(f))
                  SetVariable vars, varname, hexdata, Pointer
                  GoTo nextcommand
                Else
                  hexdata = md5.HashFile(Files(f), StartPadding:=StartRange, Padding:=20)
                End If
              ElseIf lline Like "set md4:*" Then
                md5.Algorythm = ALG_SID_MD4
                If Address <= StartRange Then
                  If StartRange <= 16 Then StartRange = 16
                  hexdata = md5.HashFile(Files(f), StartPadding:=StartRange, Padding:=endrange)
                ElseIf Len(varname) Then
                  hexdata = md5.HashFile(Files(f))
                  SetVariable vars, varname, hexdata, Pointer
                  GoTo nextcommand
                Else
                  hexdata = md5.HashFile(Files(f), StartPadding:=StartRange, Padding:=16)
                End If
              ElseIf lline Like "set md2:*" Then
                md5.Algorythm = ALG_SID_MD2
                If Address <= StartRange Then
                  If StartRange <= 16 Then StartRange = 16
                  hexdata = md5.HashFile(Files(f), StartPadding:=StartRange, Padding:=endrange)
                ElseIf Len(varname) Then
                  hexdata = md5.HashFile(Files(f))
                  SetVariable vars, varname, hexdata, Pointer
                  GoTo nextcommand
                Else
                  hexdata = md5.HashFile(Files(f), StartPadding:=StartRange, Padding:=16)
                End If
              ElseIf lline Like "set crc32:*" Then
                crc.Algorithm = CRC32
                hexdata = crc.HashFile(Files(f))
                If Address <= StartRange Then
                  If StartRange <= 4 Then StartRange = 4
                  hexdata = crc.HashFile(Files(f), StartPadding:=StartRange, Padding:=endrange)
                ElseIf Len(varname) Then
                  hexdata = crc.HashFile(Files(f))
                  SetVariable vars, varname, hexdata, Pointer
                  GoTo nextcommand
                Else
                  hexdata = crc.HashFile(Files(f), StartPadding:=StartRange, Padding:=4)
                End If
              ElseIf lline Like "set crc32big:*" Then
                crc.Algorithm = CRC32_BigEndian
                hexdata = crc.HashFile(Files(f))
                If Address <= StartRange Then
                  If StartRange <= 4 Then StartRange = 4
                  hexdata = crc.HashFile(Files(f), StartPadding:=StartRange, Padding:=endrange)
                ElseIf Len(varname) Then
                  hexdata = crc.HashFile(Files(f))
                  SetVariable vars, varname, hexdata, Pointer
                  GoTo nextcommand
                Else
                  hexdata = crc.HashFile(Files(f), StartPadding:=StartRange, Padding:=4)
                End If
              ElseIf lline Like "set crc32little:*" Then
                crc.Algorithm = CRC32_LittleEndian
                hexdata = crc.HashFile(Files(f))
                If Address <= StartRange Then
                  If StartRange <= 4 Then StartRange = 4
                  hexdata = crc.HashFile(Files(f), StartPadding:=StartRange, Padding:=endrange)
                ElseIf Len(varname) Then
                  hexdata = crc.HashFile(Files(f))
                  SetVariable vars, varname, hexdata, Pointer
                  GoTo nextcommand
                Else
                  hexdata = crc.HashFile(Files(f), StartPadding:=StartRange, Padding:=4)
                End If
              ElseIf lline Like "set crc16:*" Then
                crc.Algorithm = CRC16
                If Address <= StartRange Then
                  If StartRange <= 2 Then StartRange = 2
                  hexdata = crc.HashFile(Files(f), StartPadding:=StartRange, Padding:=endrange)
                ElseIf Len(varname) Then
                  hexdata = crc.HashFile(Files(f))
                  SetVariable vars, varname, hexdata, Pointer
                  GoTo nextcommand
                Else
                  hexdata = crc.HashFile(Files(f), StartPadding:=StartRange, Padding:=2)
                End If
              ElseIf lline Like "set adler16:*" Then
                crc.Algorithm = Adler16
                If Address <= StartRange Then
                  If StartRange <= 2 Then StartRange = 2
                  hexdata = crc.HashFile(Files(f), StartPadding:=StartRange, Padding:=endrange)
                ElseIf Len(varname) Then
                  hexdata = crc.HashFile(Files(f))
                  SetVariable vars, varname, hexdata, Pointer
                  GoTo nextcommand
                Else
                  hexdata = crc.HashFile(Files(f), StartPadding:=StartRange, Padding:=2)
                End If
              ElseIf lline Like "set adler32:*" Then
                crc.Algorithm = CRC16
                If Address <= StartRange Then
                  If StartRange <= 4 Then StartRange = 4
                  hexdata = crc.HashFile(Files(f), StartPadding:=StartRange, Padding:=endrange)
                ElseIf Len(varname) Then
                  hexdata = crc.HashFile(Files(f))
                  SetVariable vars, varname, hexdata, Pointer
                  GoTo nextcommand
                Else
                  hexdata = crc.HashFile(Files(f), StartPadding:=StartRange, Padding:=4)
                End If
'#If False Then
              ElseIf lline Like "set psid:*" Then
                hexdata = Mid$(formMain.lstPath.SelectedItem.SubItems(colPARMS), 60, 35)
                hexdata = modVB6.Replace(hexdata, "-")
                If Len(varname) Then
                  SetVariable vars, varname, hexdata, Pointer
                  GoTo nextcommand
                End If
              ElseIf lline Like "set userid:*" Then
                hexdata = Mid$(formMain.lstPath.SelectedItem.SubItems(colPARMS), 6, 8)
                hexdata = Chr$(CLng(hexdata) And &HFF&) & Chr$((CLng(hexdata) And &HFF00&) \ &H100&) & Chr$((CLng(hexdata) And &HFF0000) \ &H10000) & Chr$(0)
                hexdata = BinToHex(hexdata)
                If Len(varname) Then
                  SetVariable vars, varname, hexdata, Pointer
                  GoTo nextcommand
                End If
              ElseIf lline Like "set titleid:*" Then
                hexdata = formMain.lstPath.SelectedItem.SubItems(colTitleID)
                hexdata = BinToHex(hexdata)
                If Len(varname) Then
                  SetVariable vars, varname, hexdata, Pointer
                  GoTo nextcommand
                End If
              ElseIf lline Like "set *account*:*" Then
                hexdata = Mid$(formMain.lstPath.SelectedItem.SubItems(colPARMS), 31, 16)
                If Len(varname) Then
                  SetVariable vars, varname, hexdata, Pointer
                  GoTo nextcommand
                End If
              ElseIf lline Like "set *profile*:*" Then
                hexdata = Mid$(formMain.lstPath.SelectedItem.SubItems(colPARMS), 31, 16)
                hexdata = BinToHex(hexdata)
                If Len(varname) Then
                  SetVariable vars, varname, hexdata, Pointer
                  GoTo nextcommand
                End If
'#End If
              ElseIf lline Like "set ""*"":*" Then
                hexdata = Left$(line, pos - 2)
                hexdata = Mid$(hexdata, 6)
                hexdata = BinToHex(hexdata)
                If Len(varname) Then
                  SetVariable vars, varname, hexdata, Pointer
                  GoTo nextcommand
                End If
              ElseIf lline Like "set pointer:*" Then
                Pointer = Address
                GoTo nextcommand
              ElseIf lline Like "set [[]*[]]:*" Then
                pos = InStr(line, ":") + 1
                varname = Left$(line, pos - 2)
                varname = Mid$(varname, 6, Len(varname) - 6)
                If Len(varname) Then
                  If LCase$(Mid$(line, pos)) Like "pointer*" Then
                    SetVariable vars, varname, hexaddress, Pointer
                  ElseIf LCase$(Mid$(line, pos)) Like "reverse(*)" Then
                    parameter = Mid$(line, pos + 8, Len(line) - pos - 8)
                    hexdata = ""
                    hexdata = ParseExpression(vars, parameter)
                    hexdata = BinToHex(StrReverse(HexToBin(hexdata)))

                    SetVariable vars, varname, hexdata, Pointer
                  ElseIf LCase$(Mid$(line, pos)) Like "eof" Or LCase$(Mid$(line, pos)) Like "lastbyte" Then
                    hexdata = Hex$(FileSize - 1)
                    SetVariable vars, varname, hexdata, Pointer
                  ElseIf Mid$(line, pos) Like "0x*" Then
                    hexdata = Trim$(Mid$(line, pos + 2))
                    SetVariable vars, varname, hexdata, Pointer
                  ElseIf operator Then
                    If Address64 Then
                      hexdata = Hex64(Address64)
                    Else
                      hexdata = hexa(Address)
                    End If
                    SetVariable vars, varname, hexdata, Pointer
                  ElseIf Mid$(line, pos) Like "[[]*[]]*" Then
                    hexdata = ParseExpression(vars, Mid$(line, pos))
                    SetVariable vars, varname, hexdata, Pointer
                  Else
                    hexdata = BinToHex(Mid$(line, pos))
                    SetVariable vars, varname, hexdata, Pointer
                  End If
                  GoTo nextcommand
                End If
              Else
                GoTo nextcommand
              End If
  
              If Left$(hexdata, 2) = "0x" Then
                hexdata = Mid$(hexdata, 3)
              End If

              If fo = 0 Then
                fo = FreeFile
                Open Files(f) For Binary Shared As #fo
              End If

              c = 0
              FileSize = LOF(fo)
              For i = 1 To Len(hexdata) Step 2
                c = c + 1
                b = Ox & Mid$(hexdata, i, 2)
                If Pointer > 0 Then
                  Put #fo, Pointer + c, b
                ElseIf Address + c <= FileSize Then
                  Put #fo, Address + c, b
                End If
              Next

              StartRange = 0
              endrange = 0
              
              Close #fo
              fo = 0
            End If
          Next
        End If
      ElseIf line Like ":*" Then
        If fo Then Close #fo
        If FromLine > 0 Then Exit Do

        fo = 0
        isactive = False
        CurrentPatchName = ""
        CarryBytes = 0
        NotFound = False

        StartRange = 0
        endrange = 0
        
        Do Until Files.Count = 0
          Files.Remove 1
        Loop

        line = modVB6.Replace(line, "/", "\")
        lline = LCase$(line)

        If line Like "*\*" Then
          line = Mid$(line, 2)
          i = InStr(line, "\")
          Filename = Mid$(line, i + 1)
          line = Left$(line, i)
          If Left$(line, 1) <> "\" Then line = "\" & line
          If Not Path Like "*" & line Then
            If Path & OutDir & "\" Like "*" & line Then
              Filename = Path & OutDir & "\" & Filename
            Else
              Filename = ""
            End If
          Else
            Filename = Path & Filename
          End If
        Else
          Filename = Path & Mid$(line, 2)
        End If

        If Len(Filename) Then
          line = ExtractPath(Filename)
          Filename = Dir$(Filename, vbNormal + vbReadOnly + vbHidden + vbSystem + vbArchive)
          Do Until Filename = ""
            Files.Add line & Filename
            Filename = Dir$()
          Loop
          
          If Files.Count > 1 Then
            For i = 1 To Files.Count - 1
              ApplyPatch PatchFile, Files(1), lc + 1
              Files.Remove 1
            Next
          End If
        End If
      ElseIf line Like "[A-Za-z]*" Then
        If n >= NumSel Then Exit Do

        isactive = False
        CurrentPatchName = ""
        Key = ""
        CarryBytes = 0
        NotFound = False
        line = FixLabel(line, Group)
        lline = LCase$(line)
        With tvCheats.Nodes
          For i = 1 To .Count
            If Not .item(i).Checked Then
              'skip
            ElseIf .item(i).Text = line Then
              isactive = .item(i).Checked

              If .item(i).Parent Is Nothing Then
                'ignore null parent
              ElseIf FixLabel(.item(i).Parent.Key) <> Group Then
                isactive = False
                CurrentPatchName = ""
                CarryBytes = 0
              Else
                Key = .item(i).Parent.Key
              End If

              If isactive And Files.Count > 0 Then
                n = n + 1: Pointer = 0
                Exit For
              End If
            End If
          Next
          
          If isactive Then
            If Len(Key) Then
              If Key = line Then
                CurrentPatchName = line
              Else
                CurrentPatchName = Key & vbCrLf & line
              End If
            Else
              CurrentPatchName = line
            End If
          End If
        End With
      ElseIf line Like "[[]*[]]" Then
        If n >= NumSel Then Exit Do

        isactive = False
        CurrentPatchName = ""
        Key = ""
        CarryBytes = 0
        NotFound = False
        line = RemoveBrackets(line)
        line = FixLabel(line, Group)
        lline = LCase$(line)
        With tvCheats.Nodes
          For i = 1 To .Count
            If Not .item(i).Checked Then
              'skip
            ElseIf .item(i).Text = line Then
              isactive = .item(i).Checked
              
              If .item(i).Parent Is Nothing Then
                'ignore null parent
              ElseIf FixLabel(.item(i).Parent.Key) <> Group Then
                isactive = False
                CarryBytes = 0
              Else
                Key = .item(i).Parent.Key
              End If
              
              If isactive And Files.Count > 0 Then
                n = n + 1: Pointer = 0
                Exit For
              End If
            End If
          Next
        
          If isactive Then
            If Len(Key) Then
              If Key = line Then
                CurrentPatchName = line
              Else
                CurrentPatchName = Key & vbCrLf & line
              End If
            Else
              CurrentPatchName = line
            End If
          End If
        End With
      ElseIf line Like "[0-9]x[0-9A-Fa-f]* [0-9A-Fa-f]*" And isactive Then
parsewrite:
        If Files.Count > 0 Then
          operator = Ox & Left$(line, 1) '0 = Overwrite, 1 = OR, 2 = AND, 3 = XOR

          'address
          Address = -1 'exception address
          pos = InStr(line, " ")
          hexaddress = Trim$(Mid$(line, 3, pos - 3))
          Address = CLng(Ox & hexaddress)

          If Address >= 0 Then
            'hex data
            hexdata = Trim$(Mid$(line, pos + 1))
            hexdata = modVB6.Replace$(hexdata, " ")
            pos = InStr(hexdata, ";")
            If pos Then hexdata = Trim$(Left$(hexdata, pos - 1))

            If Left$(hexdata, 2) = "0x" Then
              hexdata = Mid$(hexdata, 3)
            End If

            For f = 1 To Files.Count
              If FileExists(Files(f)) Then
                If (GetAttr(Files(f)) And vbReadOnly) Then
                  If MsgBox(ExtractFile(Files(f)) & " is read-only." & vbCrLf & _
                            "Do you want to enable the write access to this file?", vbQuestion + vbYesNo) = vbYes Then
                    SetAttr Files(f), vbNormal
                  End If
                End If

                If (GetAttr(Files(f)) And vbReadOnly) = False Then
                  FileSize = FileLen(Files(f))

                  If fo = 0 Then
                    fo = FreeFile
                    Open Files(f) For Binary Shared As #fo
                  End If
  
                  c = 0
                  
                  If operator = 8 Then
                    databin = ""
                    databin = AllocString(FileSize - Address)
                    If Len(databin) Then Get #fo, Address + 1, databin
                  ElseIf operator = 9 Then
                    r = FreeFile
                    Open Files(f) & ".~temp" For Binary Shared As #r
                    databin = ""
                    databin = AllocString(Address)
                    If Len(databin) Then
                      Get #fo, 1, databin
                      Put #r, 1, databin
                    End If
                    databin = ""
                    databin = AllocString(FileSize - Address - ValU(Ox & hexdata & "&"))
                    If Len(databin) Then
                      Get #fo, Address + ValU(Ox & hexdata & "&") + 1, databin
                      Put #r, Address + 1, databin
                    End If
                    Close #fo
                    Close #r
                    fo = 0
                    hexdata = ""
                    FileCopy Files(f) & ".~temp", Files(f)
                    Kill Files(f) & ".~temp"
                  End If
  
                  For i = 1 To Len(hexdata) Step 2
                    c = c + 1
                    b = Ox & Mid$(hexdata, i, 2)
                    Select Case operator
                     Case 0: If Address + c <= FileSize Then Put #fo, Address + c, b
  
                     Case 1: Get #fo, Address + c, d
                             d = d Or b
                             If Address + c <= FileSize Then Put #fo, Address + c, d
                     Case 2: Get #fo, Address + c, d
                             d = d And b
                             If Address + c <= FileSize Then Put #fo, Address + c, d
                     Case 3: Get #fo, Address + c, d
                             d = d Xor b
                             If Address + c <= FileSize Then Put #fo, Address + c, d
                     Case 4: Get #fo, Address + c, d
                             d = d Eqv b
                             If Address + c <= FileSize Then Put #fo, Address + c, d
                     Case 5: Get #fo, Address + c, d
                             d = d Imp b
                             If Address + c <= FileSize Then Put #fo, Address + c, d
                     Case 6: Get #fo, Address + c, d
                             d = b Imp d
                             If Address + c <= FileSize Then Put #fo, Address + c, d
                     Case 7: 'add
                             r = Len(hexdata) \ 2
                             databin = String$(r, 0)
                             Select Case r
                              Case 1
                                Get #fo, Address + c, d
                                databin = Chr$(d + b)
                              Case 2
                                Get #fo, Address + c, databin
                                incr_value_by = CLng(Ox & hexdata) + CLng(Ox & BinToHex(databin))
                                databin = Right$("0000000" & Hex$(incr_value_by), Len(hexdata))
                                databin = Chr$(Ox & Mid$(databin, 1, 2)) & Chr$(Ox & Mid$(databin, 3, 2))
                              Case 4
                                Get #fo, Address + c, databin
                                incr_value_by = CLng(Ox & hexdata) + CLng(Ox & BinToHex(databin))
                                databin = Right$("0000000" & Hex$(incr_value_by), Len(hexdata))
                                databin = Chr$(Ox & Mid$(databin, 1, 2)) & Chr$(Ox & Mid$(databin, 3, 2)) & Chr$(Ox & Mid$(databin, 5, 2)) & Chr$(Ox & Mid$(databin, 7, 2))
                             End Select
                             If Address + c <= FileSize Then Put #fo, Address + c, databin
                             Exit For
                     Case 8: If Address > 0 Then Put #fo, Address + c, b
                    End Select
                  Next
                  'pointer = Address + c
                  
                  If operator = 8 Then
                    If Len(databin) Then Put #fo, Address + (Len(hexdata) \ 2) + 1, databin
                  End If
  
                  If Files.Count = 1 Then
                    'keep file open
                  Else
                    Close #fo
                    fo = 0
                  End If
                End If
              End If
            Next
          End If
        End If
        
        Return
      End If
    
nextcommand:
    Loop
    
    If fo Then Close #fo
    Close #fh

    Set Files = Nothing
    databin = ""
    
    If FromLine > 0 Then Exit Sub

    Screen.MousePointer = vbDefault

    If bMsg = False Then
      'skip
    ElseIf n = 1 Then
      MsgBox n & " Cheat was applied.", vbInformation
    Else
      MsgBox n & " Cheats were applied.", vbInformation
    End If
  End If
End Sub

Private Function TreeSelCount() As Long
  Dim i As Long, n As Long
  On Local Error Resume Next
  With tvCheats.Nodes
    For i = 1 To .Count
      If .item(i).Checked Then
        n = n + 1
      End If
    Next
  End With
  TreeSelCount = n
End Function

Private Function FixLabel(ByVal line As String, Optional ByRef Group As String) As String
  Dim isgroup As Boolean
  On Local Error Resume Next
  If LCase$(line) Like "group:*" Then
    line = Trim$(Mid$(line, 7))
    isgroup = True
  End If
  
  If LCase$(line) Like "default:*" Then
    line = Trim$(Mid$(line, 9))
  End If

  If LCase$(line) Like "yellow:*" Then
    line = Trim$(Mid$(line, 8))
  ElseIf LCase$(line) Like "green:*" Then
    line = Trim$(Mid$(line, 7))
  ElseIf LCase$(line) Like "blue:*" Then
    line = Trim$(Mid$(line, 6))
  ElseIf LCase$(line) Like "red:*" Then
    line = Trim$(Mid$(line, 5))
  ElseIf LCase$(line) Like "orange:*" Then
    line = Trim$(Mid$(line, 8))
  ElseIf LCase$(line) Like "cyan:*" Then
    line = Trim$(Mid$(line, 6))
  ElseIf LCase$(line) Like "magenta:*" Then
    line = Trim$(Mid$(line, 9))
  ElseIf LCase$(line) Like "gray:*" Then
    line = Trim$(Mid$(line, 6))
  End If
  
  FixLabel = line
  
  If isgroup Then
    Group = line
  End If
End Function

Public Function HexToBin(Data As String) As String
  Dim i As Long, c As Long
  On Local Error Resume Next
  HexToBin = AllocString(Len(Data) \ 2)
  For i = 1 To Len(Data) Step 2
    c = c + 1
    Mid$(HexToBin, c, 1) = Chr$(Ox & Mid$(Data, i, 2))
  Next
End Function

Private Function BinToHex(Data As String) As String
  Dim i As Long
  On Local Error Resume Next
  For i = 1 To Len(Data)
    BinToHex = BinToHex & Right$("0" & Hex$(asc(Mid$(Data, i, 1))), 2)
  Next
End Function

Public Function hexa(ByVal Value As Currency) As String
  Dim c As Currency
  Dim d(3) As String
  On Local Error Resume Next

  c = Int(Value / &H1000000) And &HFF
  d(0) = Right$("0" & Hex$(c), 2)

  Value = (Value - c * &H1000000)
  
  Do While Value >= 4294967296#
    Value = Value - 4294967296#
  Loop
  
  c = Int(Value / &H10000) And &HFF
  d(1) = Right$("0" & Hex$(c), 2)
  
  Value = Value - c * &H10000

  c = Int(Value / 256@) And &HFF
  d(2) = Right$("0" & Hex$(c), 2)
  
  Do While Value <= -2147483648#
    Value = Value + 2147483648#
  Loop

  Do While Value >= 2147483648#
    Value = Value - 2147483648#
  Loop

  c = Value And &HFF
  d(3) = Right$("0" & Hex$(c), 2)

  hexa = d(0) & d(1) & d(2) & d(3)
End Function

Public Function Deca(ByVal hexa As String) As Currency
  On Local Error Resume Next
  If Len(hexa) <= 7 Then
    Deca = CLng(Ox & hexa)
  ElseIf Len(hexa) >= 8 Then
    Deca = CCur(Ox & Mid$(hexa, Len(hexa) - 7, 2)) * &H1000000 + CLng(Ox & Right$(hexa, 6))
  End If
End Function

Private Function ParseAddress(ByRef vars As Collection, ByVal hexaddress As String, Files As Collection, Pointer As Long, Optional ByRef ppointer As Long, Optional f As Integer = 1) As String
  Dim incr_value_by As Long
  On Local Error Resume Next
  If hexaddress Like "0x*" Then hexaddress = Mid$(hexaddress, 3)
  If hexaddress Like "(*)" Then hexaddress = Hex$(CLng(RemoveBrackets(hexaddress)))
  
  If IsIn(LCase$(hexaddress), "lastbyte", "eof") Then
    hexaddress = Right$("0000000" & Hex$((FileLen(Files(f)) - 1)), 8)
    ppointer = CLng(Ox & hexaddress)
  ElseIf hexaddress Like "[[]*[]]" Then
    hexaddress = GetVariable(vars, hexaddress)
  ElseIf LCase$(hexaddress) Like "lastbyte*" Then
    incr_value_by = ValU(modVB6.Replace$(Mid$(hexaddress, 9), " ", ""))
    hexaddress = Hex$((FileLen(Files(f)) + incr_value_by - 1))
    ppointer = CLng(Ox & hexaddress)
  ElseIf LCase$(hexaddress) Like "eof*" Then
    incr_value_by = ValU(modVB6.Replace$(Mid$(hexaddress, 4), " ", ""))
    hexaddress = Hex$((FileLen(Files(f)) + incr_value_by - 1))
    ppointer = CLng(Ox & hexaddress)
  ElseIf LCase$(hexaddress) Like "pointer*" Then
    incr_value_by = 0
    incr_value_by = CLng(Ox & ParseExpression(vars, modVB6.Replace$(Mid$(hexaddress, 8), " ", "")))
    hexaddress = Hex$((Pointer + incr_value_by))
    ppointer = CLng(Ox & hexaddress)
  Else
    ppointer = Pointer + CLng(Ox & hexaddress)
  End If

  ParseAddress = hexaddress
End Function

Function ParseHexNumber(vars As Collection, ByVal databin As String) As String
  On Local Error Resume Next
  If databin Like "[+]*" Then
    databin = Mid$(databin, 2)
  ElseIf databin Like "[-]*" Then
    databin = -CLng(Ox & Mid$(databin, 2))
  End If
  
  If databin Like "0x*" Then
    Mid$(databin, 1, 2) = Ox
  ElseIf databin Like "(*)" Then
    databin = Hex$(-CLng(databin))
  ElseIf databin Like VariableMask Then
    databin = Ox & GetVariable(vars, databin)
  Else
    databin = Ox & databin
  End If
  ParseHexNumber = databin
End Function

Function ParseHex(vars As Collection, ByVal databin As String) As String
  Dim pos As Long, expression As String
  Dim negative As Boolean
  On Local Error Resume Next
  
  If databin Like "[+*/\]*" Then
    databin = Mid$(databin, 2)
  ElseIf databin Like "[-]*" Then
    databin = Mid$(databin, 2)
    negative = True
  End If
  
  If databin Like "*+*" Then
    pos = InStr(databin, "+")
    expression = Mid$(databin, pos)
    databin = Left(databin, pos - 1)
  ElseIf databin Like "*-*" Then
    pos = InStr(databin, "-")
    expression = Mid$(databin, pos)
    databin = Left(databin, pos - 1)
  End If
  
  If databin Like "0x*" Then
    databin = Mid$(databin, 3)
  ElseIf databin Like "(*)" Then
    databin = Hex$(CLng(RemoveBrackets(databin)))
  ElseIf databin Like VariableMask Then
    databin = GetVariable(vars, databin)
  End If
  
  If Len(expression) Then
    If negative Then
      ParseHex = Hex$(-ValU(Ox & databin) + ValU(ParseNumber(vars, expression)))
    Else
      ParseHex = Hex$(ValU(Ox & databin) + ValU(ParseNumber(vars, expression)))
    End If
  ElseIf negative Then
    ParseHex = Hex$(-ValU(Ox & databin))
  Else
    ParseHex = databin
  End If
End Function

Function ParseNumber(vars As Collection, ByVal databin As String) As Double
  Dim pos As Long, expression As String
  Dim negative As Boolean
  On Local Error Resume Next
  
  If databin Like "[+]*" Then
    databin = Mid$(databin, 2)
  ElseIf databin Like "[-]*" Then
    databin = Mid$(databin, 2)
    negative = True
  End If
  
  If databin Like "*+*" Then
    pos = InStr(databin, "+")
    expression = Mid$(databin, pos)
    databin = Left(databin, pos - 1)
  ElseIf databin Like "*-*" Then
    pos = InStr(databin, "-")
    expression = Mid$(databin, pos)
    databin = Left(databin, pos - 1)
  End If
  
  If databin Like "0x*" Then
    Mid$(databin, 1, 2) = Ox
  ElseIf databin Like "(*)" Then
    databin = CLng(RemoveBrackets(databin))
  ElseIf databin Like VariableMask Then
    databin = GetVariable(vars, databin)
    If databin Like "00*" Or databin Like "*[A-Fa-f]*" Then
      databin = Ox & databin
    End If
  End If
  
  If Len(expression) Then
    If negative Then
      ParseNumber = -ValU(databin) + ValU(ParseNumber(vars, expression))
    Else
      ParseNumber = ValU(databin) + ValU(ParseNumber(vars, expression))
    End If
  ElseIf negative Then
    ParseNumber = -ValU(databin)
  Else
    ParseNumber = ValU(databin)
  End If
End Function

Private Function ParseExpression(vars As Collection, ByVal expression As String) As String
  Dim lexpression As String, varname As String, databin As String
  Dim pos As Long, r As Long, c As Long
  On Local Error Resume Next

  If expression Like "&*" Then
    expression = Trim$(Mid$(expression, 2))
  ElseIf expression Like "[+]*" Then
    expression = Mid$(expression, 2)
  End If
  
  lexpression = LCase$(expression)

  If expression Like "(*)" Then
    If IsNumeric(expression) Then
      databin = Hex$(RemoveBrackets(expression))
    Else
      databin = ParseExpression(vars, RemoveBrackets(expression))
    End If
  ElseIf expression Like "[[]*[]]*" Then
    pos = InStr(expression, "]")
    varname = Mid$(expression, 2, pos - 2)
    
    databin = ""
    databin = vars(varname)
    
    expression = Trim$(Mid$(expression, pos + 1))

    If expression Like "&*" Then
      databin = databin & ParseExpression(vars, expression)
    ElseIf expression Like "[+-}*" Then
      lexpression = ParseHex(vars, expression)
      
      If Left$(lexpression, 1) = "*" Then
        databin = Right$("00000000" & Hex$(CLng(Ox & databin) * CLng(Ox & Mid$(lexpression, 2))), 8)
      ElseIf Left$(expression, 1) = "*" Then
        'databin = Right$("00000000" & Hex$(CLng(Ox & databin) * CLng(Ox & lexpression)), 8)
        databin = Hex64(CCur(Val64(databin) * Val64(lexpression)))
      ElseIf Left$(lexpression, 1) = "/" Then
        databin = Right$("00000000" & Hex$(CLng(Ox & databin) / CLng(Ox & Mid$(lexpression, 2))), 8)
      ElseIf Left$(expression, 1) = "/" Then
        databin = Right$("00000000" & Hex$(CLng(Ox & databin) / CLng(Ox & lexpression)), 8)
      Else
        databin = Right$("00000000" & Hex$(CLng(Ox & databin) + CLng(Ox & lexpression)), 8)
      End If
    End If
  ElseIf expression Like """*""*" Then
    pos = InStr(2, expression, quote)
    databin = BinToHex(Mid$(expression, 2, pos - 2))
    
    expression = Trim$(Mid$(expression, pos + 1))
    
    If expression Like "&*" Then
      databin = databin & ParseExpression(vars, expression)
    End If
  ElseIf lexpression Like "0x*" Then
    databin = Mid$(expression, 3)

  ElseIf lexpression Like "mid([[]*[]],*,*)" Then
    pos = InStr(expression, ",")
    r = InStr(pos + 1, expression, ",")
    
    varname = Trim$(Mid$(expression, 5, pos - 5))
    varname = ParseExpression(vars, varname)
    c = ParseNumber(vars, Mid$(expression, pos + 1, r - pos - 1))
    r = ParseNumber(vars, Mid$(expression, r + 1))

    databin = ""
    databin = BinToHex(Mid$(HexToBin(varname), c, r))
  
    pos = InStr(r + 1, lexpression, ")")
    expression = Trim$(Mid$(expression, pos + 1))
  
    If expression Like "&*" Then
      databin = databin & ParseExpression(vars, expression)
    End If
  ElseIf lexpression Like "mid([[]*[]],*)" Then
    pos = InStr(expression, ",")
    
    varname = Trim$(Mid$(expression, 5, pos - 5))
    varname = ParseExpression(vars, varname)
    c = ParseNumber(vars, Mid$(expression, pos + 1))

    databin = ""
    databin = BinToHex(Mid$(HexToBin(varname), c))
    
    pos = InStr(pos + 1, expression, ")")
    expression = Trim$(Mid$(expression, pos + 1))
    
    If expression Like "&*" Then
      databin = databin & ParseExpression(vars, expression)
    End If
  ElseIf lexpression Like "left([[]*[]],*)" Then
    pos = InStr(expression, ",")
    
    varname = Trim$(Mid$(expression, 6, pos - 6))
    varname = ParseExpression(vars, varname)
    c = ParseNumber(vars, Mid$(expression, pos + 1))

    databin = ""
    databin = BinToHex(Left$(HexToBin(varname), c))
  
    pos = InStr(pos + 1, expression, ")")
    expression = Trim$(Mid$(expression, pos + 1))
  
    If expression Like "&*" Then
      databin = databin & ParseExpression(vars, expression)
    End If
  ElseIf lexpression Like "right([[]*[]],*)" Then
    pos = InStr(expression, ",")
    
    varname = Trim$(Mid$(expression, 7, pos - 7))
    varname = ParseExpression(vars, varname)
    c = ParseNumber(vars, Mid$(expression, pos + 1))

    databin = ""
    databin = BinToHex(Right$(HexToBin(varname), c))
    
    pos = InStr(pos + 1, expression, ")")
    expression = Trim$(Mid$(expression, pos + 1))

    If expression Like "&*" Then
      databin = databin & ParseExpression(vars, expression)
    End If
  ElseIf lexpression Like "reverse([[]*[]])" Then
    pos = InStr(expression, ",")

    varname = Trim$(Mid$(expression, 9, pos - 9))
    varname = ParseExpression(vars, varname)

    databin = ""
    databin = BinToHex(StrReverse(HexToBin(varname)))

    pos = InStr(pos + 1, expression, ")")
    expression = Trim$(Mid$(expression, pos + 1))

    If expression Like "&*" Then
      databin = databin & ParseExpression(vars, expression)
    End If
  ElseIf lexpression Like "unicode(*)" Then
    databin = Mid$(expression, 9, Len(expression) - 9)
    databin = ParseExpression(vars, databin)
    databin = BinToHex(StrConv(HexToBin(databin), vbUnicode))

  ElseIf lexpression Like """*""*" Then
    pos = InStr(2, expression, quote)
    
    databin = BinToHex(Mid(expression, 2, pos - 2))
    expression = Trim$(Mid$(expression, pos + 1))

    If expression Like "&*" Then
      databin = databin & ParseExpression(vars, expression)
    End If
  ElseIf Len(expression) Then
    For pos = 2 To Len(expression)
      If Mid$(expression, pos, 1) = "+" Then
        Exit For
      ElseIf Mid$(expression, pos, 1) = "-" Then
        Exit For
      ElseIf Mid$(expression, pos, 1) = " " Then
        Exit For
      End If
    Next
    
    databin = ParseHex(vars, Left$(expression, pos - 1))
    expression = Trim$(Mid$(expression, pos))

    If expression Like "[+-]*" Then
      lexpression = ParseExpression(vars, expression)
      If Left(lexpression, 1) Like "+*" Then
        lexpression = Mid$(lexpression, 2)
      End If

      databin = Hex$(ValU(Ox & databin) + ValU(Ox & lexpression))
      If Left$(databin, 1) <> "-" Then
        databin = "+" & databin
      End If
    End If
  End If

  ParseExpression = databin
End Function

Private Function GetVariable(vars As Collection, varname As String) As String
  On Local Error Resume Next
  GetVariable = vars(RemoveBrackets(varname))
End Function

Private Function ConvertVariableToValue(vars As Collection, varname As String) As String
  On Local Error Resume Next
  If varname Like VariableMask Then
    ConvertVariableToValue = vars(RemoveBrackets(varname))
  Else
    ConvertVariableToValue = varname
  End If
End Function

Private Function RemoveBrackets(Value As String) As String
  On Local Error Resume Next
  RemoveBrackets = Mid$(Value, 2, Len(Value) - 2)
End Function

Private Sub SetVariable(vars As Collection, varname, Value As String, Pointer As Long)
  On Local Error Resume Next
  If LCase$(varname) = "pointer" Then
    Pointer = CLng(ParseHexNumber(vars, Value))
  Else
    vars.Remove varname
    vars.Add Value, varname
  End If
End Sub

Private Function GetParameter(line As String, pos As Long, ByVal delim1 As String, Optional ByVal delim2 As String = "", Optional delim3 As String = " ", Optional default As String) As String
  Dim i As Long, endpos As Long
  Dim ldelim
  On Local Error Resume Next
  GetParameter = default

  i = InStr(pos, line, delim1, vbTextCompare)
  If i Then
    If Len(delim2) Then
      endpos = InStr(i, line, delim2, vbTextCompare)
    Else
      pos = i + Len(delim1)
      GetParameter = Mid$(line, pos)
      pos = 0
      Exit Function
    End If

    If endpos Then
      pos = i + Len(delim1)
      GetParameter = Mid$(line, pos, endpos - pos)
      pos = pos + Len(GetParameter)
      Exit Function
    Else
      endpos = InStr(i, line, delim3, vbTextCompare)
      If endpos Then
        pos = i + Len(delim1)
        GetParameter = Mid$(line, pos, endpos - pos)
        pos = pos + Len(GetParameter)
      Else
        endpos = InStr(i, line, ":")
        If endpos Then
          pos = i + Len(delim1)
          GetParameter = Mid$(line, pos, endpos - pos)
          pos = pos + Len(GetParameter)
        Else
          pos = i + Len(delim1)
          GetParameter = Mid$(line, pos)
          pos = 0
        End If
      End If
      Exit Function
    End If
  End If
End Function

Private Function Hex64(num As Currency, Optional bits32 As Boolean = True) As String
  Dim i As Integer, base As Double
  Dim n As Currency, c As Long
  On Local Error Resume Next
  n = num
  c = 0
  For i = 63 To 32 Step -1
    base = 2@ ^ i
    If n >= base Then
      c = c + 2& ^ (i - 32)
      n = n - base
    End If
  Next
  If n And &H80000000 Then n = n - (2@ ^ 31)
  
  If bits32 Then
    Hex64 = Right$("00000000" & Hex$(n), 8)
  Else
    Hex64 = Right$("00000000" & Hex$(c), 8) & Right$("00000000" & Hex$(n), 8)
  End If
End Function

 


